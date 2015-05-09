// IrpsimEngine.h
#pragma once

#include <Windows.h>
#include <msclr/marshal.h>
#include "options.h"

using namespace System;
using namespace msclr::interop;

namespace IrpsimEngine {

	public ref class TestClass
	{
		CMOption *op;
	public:
		TestClass() {
			this->op = new CMOption(L"TestNameAgain",L"TestValue");
		}

		~TestClass() {
			Console::WriteLine(L"Deleting TestClass");
			delete this->op;
		}

		String^ TestFunc() {
			return marshal_as<String^>(this->op->GetName().c_str());
		}

		// TODO: Add your methods for this class here.
	};
}
