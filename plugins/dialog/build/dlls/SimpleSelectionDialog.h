#pragma once

#include "DialogTemplate.h"

namespace DialogLib {

	/// <summary>
	/// Summary for SimpleSelectionDialog
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SimpleSelectionDialog : public DialogTemplate
	{
	public:
		SimpleSelectionDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			this->_helpText = gcnew System::String("Select an item and press ok.");
			this->_item = gcnew System::Collections::ArrayList();

			this->comboBoxSelection->DataSource = this->_item;
		}

	public: virtual int AddItem(System::String ^itemID, System::String ^itemValue) override;
	public: virtual bool AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue) override;
	public: virtual bool RemoveItem(System::String ^itemID, int ordinalNumber) override;
	public: virtual bool RemoveItems(System::String ^itemID) override;

	protected:
		System::Collections::ArrayList^ _item;
		System::Collections::ArrayList^ _description;

	protected: 
		System::Collections::ArrayList^ _devicePath;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SimpleSelectionDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  buttonOK;
	protected: 
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::ComboBox^  comboBoxSelection;
	private: System::Windows::Forms::Label^  labelSelection;





	private: System::ComponentModel::IContainer^  components;


	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->buttonOK = (gcnew System::Windows::Forms::Button());
			this->buttonCancel = (gcnew System::Windows::Forms::Button());
			this->comboBoxSelection = (gcnew System::Windows::Forms::ComboBox());
			this->labelSelection = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// buttonOK
			// 
			this->buttonOK->Location = System::Drawing::Point(287, 87);
			this->buttonOK->Name = L"buttonOK";
			this->buttonOK->Size = System::Drawing::Size(75, 23);
			this->buttonOK->TabIndex = 0;
			this->buttonOK->Text = L"OK";
			this->buttonOK->UseVisualStyleBackColor = true;
			this->buttonOK->Click += gcnew System::EventHandler(this, &SimpleSelectionDialog::buttonOK_Click);
			// 
			// buttonCancel
			// 
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->buttonCancel->Location = System::Drawing::Point(368, 87);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(75, 23);
			this->buttonCancel->TabIndex = 1;
			this->buttonCancel->Text = L"Cancel";
			this->buttonCancel->UseVisualStyleBackColor = true;
			this->buttonCancel->Click += gcnew System::EventHandler(this, &SimpleSelectionDialog::buttonCancel_Click);
			// 
			// comboBoxSelection
			// 
			this->comboBoxSelection->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxSelection->Location = System::Drawing::Point(13, 41);
			this->comboBoxSelection->Name = L"comboBoxSelection";
			this->comboBoxSelection->Size = System::Drawing::Size(428, 21);
			this->comboBoxSelection->TabIndex = 4;
			// 
			// labelSelection
			// 
			this->labelSelection->AutoSize = true;
			this->labelSelection->Location = System::Drawing::Point(14, 15);
			this->labelSelection->Name = L"labelSelection";
			this->labelSelection->Size = System::Drawing::Size(54, 13);
			this->labelSelection->TabIndex = 6;
			this->labelSelection->Text = L"Selection:";
			this->labelSelection->Click += gcnew System::EventHandler(this, &SimpleSelectionDialog::labelFriendlyName_Click);
			// 
			// SimpleSelectionDialog
			// 
			this->AcceptButton = this->buttonOK;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->buttonCancel;
			this->ClientSize = System::Drawing::Size(453, 126);
			this->Controls->Add(this->labelSelection);
			this->Controls->Add(this->comboBoxSelection);
			this->Controls->Add(this->buttonCancel);
			this->Controls->Add(this->buttonOK);
			this->Name = L"SimpleSelectionDialog";
			this->Text = L"SimpleSelectionDialog";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void buttonOK_Click(System::Object^  sender, System::EventArgs^  e)
		 {
			 this->_selectedOption = this->comboBoxSelection->SelectedIndex;
			 this->Close();
		 }
private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e)
		 {
			 this->_selectedOption = -1;
			 this->Close();
		 }
private: System::Void labelFriendlyName_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}
