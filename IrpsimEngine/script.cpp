// script.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMScript implements IRPSIM's scripting functionality
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "script.h"
#include "category.h"
#include "varcol.h"
#include "simulat.h"
#include "irpapp.h"
#include "notify.h"

#include "cmlib.h"
#include "token.h"

#include <iomanip>

wchar_t* CMScript::error_strings[] =
{
	L"syntax error",
   L"missing bracket",
   L"bad allocation rule",
   L"missing script",
   L"missing ENDIF",
   L"misplaced ELSE",
   L"misplaced ENDIF"
};

wchar_t* CMScript::script_tokens[] = {
	L"evaluate",
   L"cut",
   L"gain",
   L"loss",
   L"put",
   L"take",
   L"transfer",
   L"shift",
   L"if",
   L"else",
   L"endif",
   L"stage",
   L"category",
   L"node",
   L"limit",
   L"using",
   L"demand",
   L"supply",
   L"storage",
   L"shortage",
   L"from",
   L"to",
   L"do",
   L"ignorebalance",
   L"end",
   L"@LINE",
   L"@ERROR",
   0
};

wchar_t* CMScript::header = L"#SCRIPT";
wchar_t* CMScript::footer = L"#END";

const wchar_t* CMScript::IsA() { return L"CMScript"; }

CMScript::CMScript(int id) :
CMIrpObject(id),
state(0),
tm(0),
sim(0),
script(),
stagenames(),
categorynames(),
nodenames(),
expressions(),
categories(),
nodes(),
stages(0)
{
}

CMScript::~CMScript()
{
   lines.Reset(1);
	reset();
}

void CMScript::reset()
{
	script.Reset(1);
	categories.Reset(1);
   stagenames.Reset(1);
   categorynames.Reset(1);
   nodenames.Reset(1);
	nodes.Reset(1);
	expressions.ResetAndDestroy(1);
   if (stages!=0) delete [] stages;
   stages=0;
	state=0;
   tm=0;
   sim=0;
}

int CMScript::evaluate_token(const CMString& token)
{
	for (int i=0;i<=(TokEnd-TokEvaluate);i++)
   	if (token==script_tokens[i])
      	return TokEvaluate+i;
	return TokError;
}

int CMScript::resolve_pointers()
{
	int nif=0,nelse=0,nendif=0,nline=0;
	for (int i=0;i<script.Count();i++) {
   	if (script[i] == TokIf) {
      	nif++;
			int elseindex=-1;
         script[i+2]=-1;
         script[i+3]=-1;
			int tok_count=0;
         for (int j=i+1;j<script.Count();j++) {
         	if (script[j]==TokIf)
            	tok_count++;
         	else if (script[j]==TokElse && !tok_count) {
					elseindex=j+1;
            	script[i+2]=elseindex;
				}
         	else if (script[j]==TokEndif) {
            	if (!tok_count) {
               	script[i+3]=j;
                  if (elseindex>0) script[elseindex]=j;
                  break;
               }
               else
               	tok_count--;
            }
         }
      }
      else if (script[i]==TokElse)
      	nelse++;
      else if (script[i]==TokEndif)
      	nendif++;
      else if (script[i]==TokLine)
      	script[i+1]=nline++;
   }
	if (nif<nendif)
		CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XMisplacedEndif]) + L" script <" + name + L">");
	if (nif<nelse)
		CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XMisplacedElse]) + L" script <" + name + L">");
	if (nif>nendif)
		CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XMissingEndif]) + L" script <" + name + L">");
	return ((nif==nendif) && (nif>=nelse)) ? 1 : 0;
}

int CMScript::parse_line(const CMString &aline,CMIrpApplication& a)
{
	static const wchar_t* delims = L" \t\r\n";
	CMString line = stripends(aline);
	size_t comment_found = line.find(L"//");
   if (comment_found != CM_NPOS)
   	line = line.substr(0,comment_found);
	CMTokenizer next(line);
	if (line.is_null() || line[0]==L'*')
		return ParseComment;
   CMString token;
   int ret = ParseOK;
   int firsttoken=1;
   while (!(token=next(delims)).is_null()) {
   	int tok = evaluate_token(token);
      if (tok>TokEnd) {
      	ret = ParseError;
		CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XSyntaxError]) + L" script <" + name + L"> line <" + line + L"> token <" + token + L">");
      }
		if (tok==TokDo) {
			CMScript* sfound = a.FindScript(stripends(next(L"\r\n")));
         if (sfound == 0) {
	      	ret = ParseError;
			CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XMissingScript]) + L" script <" + name + L"> line <" + line + L">");
         }
		 else
         	parse_lines(*sfound,a);
         break;
      }
      if (firsttoken) {
			script.Add(TokLine);
			script.Add(-1);
         firsttoken=0;
      }
      script.Add(tok);
      if (tok==TokCategory || tok==TokStage || tok==TokNode) {
		   unsigned short loc;
			CMVSmallArray<CMString>* names = &categorynames;
         if (tok==TokStage) names = &stagenames;
         else if (tok==TokNode) names = &nodenames;
			token = next(delims);
			if (!names->Contains(token,&loc)) {
         	loc=names->Count();
            names->Add(token);
			}
         script.Add(loc);
      }
		else if (tok==TokUsing) {
			token = next(delims);
         int rule = CMAllocationUnit::TranslateAllocationRule(token);
         if (rule == CMAllocationUnit::Preserve) {
	      	ret = ParseError;
			CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XBadAllocationRule]) + L" script <" + name + L"> line <" + line + L">");
         }
			script.Add(rule);
      }
		else if (tok==TokLimit || tok==TokIf) {
         int beg = line.find(L'{');
         int end = line.find(L'}');
			if (beg != CM_NPOS) {
         	if (end==CM_NPOS) {
		      	ret = ParseError;
				CMNotifier::Notify(CMNotifier::ERROR, CMString(error_strings[XMissingBracket]) + L" script <" + name + L"> line <" + line + L">");
               end = line.length()-1;
            }
            token = line.substr(beg+1,end-beg-1);
            if (end<(int)line.length()-1)
	            next.Reset(line.substr(end+1,line.length()-end));
         }
         else
         	token = next(delims);
         script.Add(expressions.Count());
         expressions.Add(new CMExpression(token.c_str()));
         if (tok==TokIf) {
         	script.Add(-1);
         	script.Add(-1);
         }
			if (beg != CM_NPOS && end >= (int)line.length()-1)
   	     	break;
      }
      else if (tok==TokElse)
	     	script.Add(-1);
	}
   return ret;
}

/*
	void Transfer(int fromrange,void* fromtarget,int torange,void* totarget,double limit,int rule);
*/

int CMScript::evaluatestage(int range,int index,int limindex,int rule)
{
	if (stages && index>=0 && index<stagenames.Count())
 		for (unsigned short i=0;i<stages[index].Count();i++)
	   	stages[index][i]->GetValue(tm);
	return 1;
}


int CMScript::evaluatenodes(int range,int index,int limindex,int rule,int nodetype)
{
	double limit = (limindex>=0) ? expressions[limindex]->Evaluate(tm) : CM_BIGDOUBLE;
   if (range==fAll) {
		double amt=0;
		for (unsigned short i=0;CMNode::GetNode(i)!=0 && amt<limit;i++)
      	if (CMNode::GetNode(i)->GetType() == nodetype)
         	amt += CMNode::GetNode(i)->Evaluate(limit-amt);
	}
   else if (range==fCategory)
		categories[index]->Evaluate(limit,rule);
   else if (range==fNode)
   	nodes[index]->Evaluate(limit);
   return 1;
}

int CMScript::cutback(int range,int index,int limindex,int rule,int ignorebalance,int nodetype)
{
	double limit = (limindex>=0) ? expressions[limindex]->Evaluate(tm) : CM_BIGDOUBLE;
	double ns = (nodetype==CMAllocationUnit::aDemand ? -1 : 1)* vnetsurplus->GetValue(tm);
	double attempt = ignorebalance ? limit : ns;
	attempt = cmmin(attempt,limit);

   if (range==fAll) {
     	double amtcut = 0;
		for (unsigned short i=0;CMNode::GetNode(i)!=0&&amtcut<attempt;i++) {
      	if (CMNode::GetNode(i)->GetType() == nodetype)
         	amtcut += CMNode::GetNode(i)->CutBack(attempt-amtcut);
      }
   }
   else if (range==fCategory)
		categories[index]->CutBack(attempt,rule,ignorebalance,nodetype);
   else if (range==fNode)
   	nodes[index]->CutBack(attempt);
   return 1;
}

int CMScript::gain(int range,int index)
{
   if (range==fAll) {
		for (unsigned short i=0;CMNode::GetNode(i)!=0;i++)
      	if (CMNode::GetNode(i)->GetType() == CMAllocationUnit::aStorage)
         	CMNode::GetNode(i)->Gain();
   }
   else if (range==fCategory)
		categories[index]->Gain();
   else if (range==fNode)
   	nodes[index]->Gain();
   return 1;
}

int CMScript::loss(int range,int index)
{
   if (range==fAll) {
		for (unsigned short i=0;CMNode::GetNode(i)!=0;i++)
      	if (CMNode::GetNode(i)->GetType() == CMAllocationUnit::aStorage)
         	CMNode::GetNode(i)->Loss();
   }
   else if (range==fCategory)
		categories[index]->Loss();
   else if (range==fNode)
   	nodes[index]->Loss();
   return 1;
}

int CMScript::put(int range,int index,int limindex,int rule,int ignorebalance)
{
	double limit = (limindex>=0) ? expressions[limindex]->Evaluate(tm) : CM_BIGDOUBLE;
	double ns = vnetsurplus->GetValue(tm);
	double attempt = ignorebalance ? limit : ns;
	attempt = cmmin(attempt,limit);

	if (range==fAll) {
     	double amtput = 0;
		for (unsigned short i=0;CMNode::GetNode(i)!=0&&amtput<attempt;i++) {
      	if (CMNode::GetNode(i)->GetType() == CMAllocationUnit::aStorage)
         	amtput += CMNode::GetNode(i)->Put(attempt-amtput,0);
      }
   }
   else if (range==fCategory)
		categories[index]->Put(attempt,rule,ignorebalance,0);
   else if (range==fNode)
   	nodes[index]->Put(attempt,0);
   return 1;
}

int CMScript::take(int range,int index,int limindex,int rule,int ignorebalance)
{
	double limit = (limindex>=0) ? expressions[limindex]->Evaluate(tm) : CM_BIGDOUBLE;
	double ns = -vnetsurplus->GetValue(tm);
	double attempt = ignorebalance ? limit : ns;
	attempt = cmmin(attempt,limit);

	if (range==fAll) {
     	double amttake = 0;
		for (unsigned short i=0;CMNode::GetNode(i)!=0&&amttake<attempt;i++) {
      	if (CMNode::GetNode(i)->GetType() == CMAllocationUnit::aStorage)
         	amttake += CMNode::GetNode(i)->Take(attempt-amttake,0);
      }
   }
   else if (range==fCategory)
		categories[index]->Take(attempt,rule,ignorebalance,0);
   else if (range==fNode)
   	nodes[index]->Take(attempt,0);
   return 1;
}

int CMScript::transfer(int ,int,int,int,int)
{
   return 1;
}

int CMScript::shift(int fromrange,int fromindex,int torange,int toindex,int limindex)
{
	double limit = (limindex>=0) ? expressions[limindex]->Evaluate(tm) : CM_BIGDOUBLE;
   double putability=0,takeability=0;

   if (fromrange==fCategory)		takeability = categories[fromindex]->TakeAbility();
   else if (fromrange==fNode)   	takeability = nodes[fromindex]->TakeAbility();
   if (torange==fCategory)   		putability = categories[toindex]->PutAbility();
   else if (torange==fNode)     	putability = nodes[toindex]->PutAbility();

   double amttoshift = cmmin(putability,takeability);
   amttoshift = cmmin(amttoshift,limit);

   if (fromrange==fCategory)		categories[fromindex]->Take(amttoshift,-1,1,1);
   else if (fromrange==fNode)   	nodes[fromindex]->Take(amttoshift,1);
   if (torange==fCategory)   		categories[toindex]->Put(amttoshift,-1,1,1);
   else if (torange==fNode)     	nodes[toindex]->Put(amttoshift,1);

   return 1;
}

int CMScript::SetSimulationContext(CMSimulation* s,CMIrpApplication* app)
{
	unsigned short i;

	RemoveSimulationContext();

   if (Fail())
   	return 0;

	sim = s;
	CMNode::ResetAggregateVariables(true);

	vnetsurplus = CMVariable::Find(CMVariableTypes::AggStringFromInt(CMVariableTypes::aNetSurplus));

	for (i=0;i<expressions.Count();i++)
   	expressions[i]->UpdateVariableLinks();

	CMVariableIterator iter;
	CMVariable* v;
	while ((v = iter()) != 0) {
		const wchar_t* pName = v->GetName().c_str();
		int isSelected = v->GetState(CMVariable::vsSelected);
		if ((v->GetState(CMVariable::vsSelected) == TRUE) && (v->IsType(L"supply") || v->IsType(L"demand") || v->IsType(L"storage")))
			CMNode::AddNode(v->GetName());
	}

 	for (i=0;i<nodenames.Count();i++) {
   	  CMNode* n = CMNode::AddNode(nodenames[i]);
      if (n==0) {
		  CMNotifier::Notify(CMNotifier::ERROR, CMString(L"undefined node in script <") + name + L">: " + nodenames[i]);
         state |= sFail;
      }
		nodes.AddAt(i,n);
   }

	for (i=0;i<categorynames.Count();i++) {
		CMCategory* cat = app->FindCategory(categorynames[i]);
      if (cat==0) {
		  CMNotifier::Notify(CMNotifier::ERROR, CMString(L"undefined category in script <") + name + L">: " + categorynames[i]);
         state |= sFail;
      }
      else {
      	if (!cat->Initialize())
	         state |= sFail;
      }
	  	categories.AddAt(i,cat);
   }

	if (stages) {
		CMVariableIterator iter;
		CMVariable* v;
		while ((v=iter())!=0) {
			if (v->GetState(CMVariable::vsSystem)==TRUE)
      		continue;
			if (v->GetState(CMVariable::vsSelected)==TRUE) {
				for (unsigned short i=0;i<stagenames.Count();i++) {
      	   	if (v->IsType(stagenames[i])) {
               	stages[i].Add(v);
         	   	break;
               }
            }
	      }
   	}
   }

   if (Fail())
   	RemoveSimulationContext();

   return (Fail()) ? 0 : 1;
}

void CMScript::RemoveSimulationContext()
{
	nodes.Reset(1);
	categories.Reset(1);
   for (unsigned short i=0;stages && i<stagenames.Count();i++)
	  	stages[i].Reset(1);
	CMNode::DestroyNodes();
   sim=0;
}

int CMScript::Run(CMTimeMachine* t)
{
	int operation,which,fromrange,fromindex,torange,toindex,limindex,rule,ignorebalance;
   int endscript = 0;

   tm = t;

	if (Fail()) {
		CMNotifier::Notify(CMNotifier::ERROR, CMString(L"can't run script <") + name + L">: " + error_strings[XSyntaxError]);
		return 0;
   }


   CMCategory::InitTimeStep(t);
	CMNode::InitAggregateVariables(t);
   CMNode::InitTimeStep(t);

   for (unsigned short i=0;i<script.Count() && !endscript;i++) {
		which=fromindex=torange=toindex=limindex=rule=-1;
      fromrange=fAll;
		while (script[i]==TokLine || script[i]>=0) i++;
		operation = script[i];
		if (operation==TokIf) {
         if (expressions[script[++i]]->Evaluate(tm) == 0)
	         i = script[i+1]>0 ? script[i+1] : script[i+2];
         else
         	i+=2;
			continue;
      }
      else if (operation==TokElse) {
         i++;
      	if (script[i]>0) i = script[i];
         continue;
      }
		else if (operation==TokEnd)
      	break;
		int fromtoflag=0;
      ignorebalance=0;
      for (;i<script.Count() && script[i]!=TokLine;i++) {
			switch (script[i]) {
         	case TokDemand: which=fDemand; break;
         	case TokSupply: which=fSupply; break;
         	case TokTransfer: which=fTransfer; break;
         	case TokStage:
            	which=fStage;
               fromindex = script[++i];
               break;
         	case TokShortage: torange=fShortage; break;
         	case TokStorage:
            	if (fromtoflag==1)
               	torange=fAll;
               break;
				case TokFrom: fromtoflag=0;break;
				case TokTo: fromtoflag=1;break;
				case TokCategory:
				case TokNode:
	           	if (fromtoflag==0) {
		           	fromrange = (script[i]==TokCategory) ? fCategory : fNode;
               	fromindex=script[++i];
               }
               else {
		           	torange = (script[i]==TokCategory) ? fCategory : fNode;
               	toindex=script[++i];
               }
            	break;
            case TokLimit: limindex=script[++i]; break;
            case TokUsing: rule=script[++i]; break;
            case TokIgnore: ignorebalance=1;break;
         }
      }
		switch (operation) {
			case TokEvaluate:
   			if (which==fStage) evaluatestage(fromrange,fromindex,limindex,rule);
   			else if (which==fDemand) evaluatenodes(fromrange,fromindex,limindex,rule,CMAllocationUnit::aDemand);
   			else if (which==fSupply) evaluatenodes(fromrange,fromindex,limindex,rule,CMAllocationUnit::aSupply);
   			else if (which==fTransfer) evaluatenodes(fromrange,fromindex,limindex,rule,CMAllocationUnit::aTransfer);
				break;
			case TokCut:
   			if (which==fDemand) cutback(fromrange,fromindex,limindex,rule,ignorebalance,CMAllocationUnit::aDemand);
   			else if (which==fSupply) cutback(fromrange,fromindex,limindex,rule,ignorebalance,CMAllocationUnit::aSupply);
   			else if (which==fTransfer) cutback(fromrange,fromindex,limindex,rule,ignorebalance,CMAllocationUnit::aTransfer);
            break;
			case TokGain:
         	gain(fromrange,fromindex);
            break;
			case TokLoss:
         	loss(fromrange,fromindex);
            break;
			case TokPut:
			   put(fromrange,fromindex,limindex,rule,ignorebalance);
				break;
			case TokTake:
			   take(fromrange,fromindex,limindex,rule,ignorebalance);
				break;
//			case TokTransfer:
//				transfer(fromrange,fromindex,torange,toindex,limindex);
//				break;
			case TokShift:
				shift(fromrange,fromindex,torange,toindex,limindex);
				break;
      }
	}

   CMNode::EndTimeStep(t);

   return 1;
}

wistream& CMScript::read(wistream& s)
{
	lines.Reset(1);
	reset();
	CMString line;
	while (!s.eof()) {
		line.read_line(s);
   		line = stripends(line);
		if (line.is_null())
      		continue;
		else if (line(0,wcslen(header)) == header)
      		name = stripends(CMString(line.c_str()+wcslen(header)));
		else if (line(0,wcslen(footer)) == footer)
   			break;
		else
			lines.Add(line);
   }
   return s;
}

void CMScript::parse_lines(CMScript& s,CMIrpApplication& a)
{
	CMString line;
	int oldcase = CMString::set_case_sensitive(0);
   for (unsigned short i=0;i<s.lines.Count();i++) {
		if (parse_line(s.lines[i],a) == ParseError)
	     	state |= sFail;
   }
	CMString::set_case_sensitive(oldcase);
}

void CMScript::Parse(CMIrpApplication& a)
{
	CMString line;
	int begin = 0;
	int oldcase = CMString::set_case_sensitive(0);
   reset();
	parse_lines(*this, a);
	if (!resolve_pointers())
   	state |= sFail;
	int nstage = stagenames.Count();
   if (nstage > 0)
   	stages = new CMPSmallArray<CMVariable>[nstage];
	CMString::set_case_sensitive(oldcase);
}

wostream& CMScript::write(wostream& s)
{
	s << setiosflags(ios::left) << header << L' ' << name << ENDL;
   int ntabs=0;
   int begline=1;
	for (int i=0;i<script.Count();i++) {
		int tok = script[i];
      if (tok==TokElse||tok==TokEndif) ntabs--;
		if (tok != TokLine) {
      	for (int j=0;begline&&j<ntabs;j++) s << L"   ";
	      s << script_tokens[tok-TokEvaluate] << L' ';
         begline=0;
      }
      else
      	begline=1;
		switch (tok) {
      	case TokCategory:
            s << categorynames[script[++i]] << L' '; break;
      	case TokStage:
            s << stagenames[script[++i]] << L' '; break;
      	case TokNode:
            s << nodenames[script[++i]] << L' '; break;
			case TokLimit:
         	s << L'{' << *expressions[script[++i]] << L"} "; break;
			case TokIf:
         	s << L'{' << *expressions[script[++i]] << L"} ";
            i+=2;
            ntabs++;
            break;
			case TokElse:
            i++;
            ntabs++;
            break;
         case TokUsing:
				s << CMAllocationUnit::TranslateAllocationRule(script[++i]) << L' '; break;
         case TokLine:
         	if (script[++i] > 0) s << ENDL;
            break;
      }
   }
	s << ENDL << footer << ENDL;
   return s;
}

