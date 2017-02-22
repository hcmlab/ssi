#pragma once

//using namespace System;
//using namespace System::ComponentModel;
//using namespace System::Collections;
//using namespace System::Windows::Forms;
//using namespace System::Data;
//using namespace System::Drawing;


namespace DialogLib {

	/// <summary>
	/// Summary for DialogTemplate
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class DialogTemplate : public System::Windows::Forms::Form
	{
	public:
		DialogTemplate(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			_selectedOption = -1;
			_helpText = gcnew System::String("No additional Help available");

			this->_returnString = gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();
			this->_returnDouble = gcnew System::Collections::Generic::Dictionary<System::String^, double>();
			this->_returnFloat = gcnew System::Collections::Generic::Dictionary<System::String^, float>();
			this->_returnInt = gcnew System::Collections::Generic::Dictionary<System::String^, int>();
		}

	public: int _selectedOption;
	public: System::String^ _helpText;

	public: virtual int AddItem(System::String ^itemID, System::String ^itemValue);
	public: virtual bool AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue);
	public: virtual bool RemoveItem(System::String ^itemID, int ordinalNumber);
	public: virtual bool RemoveItems(System::String ^itemID);

	public: System::Collections::Generic::Dictionary<System::String^, System::String^>^	_returnString;
	public: System::Collections::Generic::Dictionary<System::String^, double>^			_returnDouble;
	public: System::Collections::Generic::Dictionary<System::String^, float>^			_returnFloat;
	public: System::Collections::Generic::Dictionary<System::String^, int>^				_returnInt;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DialogTemplate()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->SuspendLayout();
			// 
			// DialogTemplate
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(389, 322);
			this->HelpButton = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"DialogTemplate";
			this->Text = L"DialogTemplate";
			this->HelpButtonClicked += gcnew System::ComponentModel::CancelEventHandler(this, &DialogTemplate::DialogTemplate_HelpButtonClicked);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void DialogTemplate_HelpButtonClicked(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e) {
				 System::Windows::Forms::MessageBox::Show(_helpText, "Help for this Dialog", System::Windows::Forms::MessageBoxButtons::OK);
			 }
	};
}
