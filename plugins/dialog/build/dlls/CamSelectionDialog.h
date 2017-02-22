#pragma once

//using namespace System;
//using namespace System::ComponentModel;
//using namespace System::Collections;
//using namespace System::Windows::Forms;
//using namespace System::Data;
//using namespace System::Drawing;

#include "DialogTemplate.h"

namespace DialogLib {

	/// <summary>
	/// Summary for CamSelectionDialog
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class CamSelectionDialog : public DialogTemplate
	{
	public:
		CamSelectionDialog(void)
		{
			InitializeComponent();

			//
			//TODO: Add the constructor code here
			//
			this->_helpText = gcnew System::String("TODO");
			this->_friendlyName = gcnew System::Collections::ArrayList();
			this->_description = gcnew System::Collections::ArrayList();
			this->_devicePath = gcnew System::Collections::ArrayList();
			this->_friendlyName->Add("None");
			this->_description->Add("");
			this->_devicePath->Add("");

			this->comboBoxFriendlyName->DataSource = this->_friendlyName;
			this->comboBoxDescription->DataSource = this->_description;
		}

	public: virtual int AddItem(System::String ^itemID, System::String ^itemValue) override;
	public: virtual bool AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue) override;
	public: virtual bool RemoveItem(System::String ^itemID, int ordinalNumber) override;
	public: virtual bool RemoveItems(System::String ^itemID) override;

	protected:
		System::Collections::ArrayList^ _friendlyName;
		System::Collections::ArrayList^ _description;

	protected: 
		System::Collections::ArrayList^ _devicePath;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~CamSelectionDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  buttonOK;
	protected: 
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::TextBox^  textBoxDevicePath;

	private: System::Windows::Forms::Label^  labelDevicePath;
	private: System::Windows::Forms::ComboBox^  comboBoxFriendlyName;
	private: System::Windows::Forms::ComboBox^  comboBoxDescription;
	private: System::Windows::Forms::Label^  labelFriendlyName;
	private: System::Windows::Forms::Label^  labelDescription;



	private: System::Windows::Forms::ToolTip^  toolTipDevicePath;

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
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(CamSelectionDialog::typeid));
			this->buttonOK = (gcnew System::Windows::Forms::Button());
			this->buttonCancel = (gcnew System::Windows::Forms::Button());
			this->textBoxDevicePath = (gcnew System::Windows::Forms::TextBox());
			this->labelDevicePath = (gcnew System::Windows::Forms::Label());
			this->comboBoxFriendlyName = (gcnew System::Windows::Forms::ComboBox());
			this->comboBoxDescription = (gcnew System::Windows::Forms::ComboBox());
			this->labelFriendlyName = (gcnew System::Windows::Forms::Label());
			this->labelDescription = (gcnew System::Windows::Forms::Label());
			this->toolTipDevicePath = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->SuspendLayout();

			// 
			// buttonOK
			// 
			this->buttonOK->Location = System::Drawing::Point(284, 235);
			this->buttonOK->Name = L"buttonOK";
			this->buttonOK->Size = System::Drawing::Size(75, 23);
			this->buttonOK->TabIndex = 0;
			this->buttonOK->Text = L"OK";
			this->buttonOK->UseVisualStyleBackColor = true;
			this->buttonOK->Click += gcnew System::EventHandler(this, &CamSelectionDialog::buttonOK_Click);
			// 
			// buttonCancel
			// 
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->buttonCancel->Location = System::Drawing::Point(365, 235);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(75, 23);
			this->buttonCancel->TabIndex = 1;
			this->buttonCancel->Text = L"Cancel";
			this->buttonCancel->UseVisualStyleBackColor = true;
			this->buttonCancel->Click += gcnew System::EventHandler(this, &CamSelectionDialog::buttonCancel_Click);
			// 
			// textBoxDevicePath
			// 
			this->textBoxDevicePath->Location = System::Drawing::Point(12, 145);
			this->textBoxDevicePath->Multiline = true;
			this->textBoxDevicePath->Name = L"textBoxDevicePath";
			this->textBoxDevicePath->ReadOnly = true;
			this->textBoxDevicePath->Size = System::Drawing::Size(428, 74);
			this->textBoxDevicePath->TabIndex = 2;
			this->toolTipDevicePath->SetToolTip(this->textBoxDevicePath, L"The unique DevicePath of the Video Device currently selected.\nYou can select and " 
				L"copy it to the clipboard, too.");
			// 
			// labelDevicePath
			// 
			this->labelDevicePath->AutoSize = true;
			this->labelDevicePath->Location = System::Drawing::Point(12, 126);
			this->labelDevicePath->Name = L"labelDevicePath";
			this->labelDevicePath->Size = System::Drawing::Size(69, 13);
			this->labelDevicePath->TabIndex = 3;
			this->labelDevicePath->Text = L"Device Path:";
			// 
			// comboBoxFriendlyName
			// 
			this->comboBoxFriendlyName->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxFriendlyName->Location = System::Drawing::Point(11, 35);
			this->comboBoxFriendlyName->Name = L"comboBoxFriendlyName";
			this->comboBoxFriendlyName->Size = System::Drawing::Size(428, 21);
			this->comboBoxFriendlyName->TabIndex = 4;
			this->comboBoxFriendlyName->SelectedIndexChanged += gcnew System::EventHandler(this, &CamSelectionDialog::comboBoxFriendlyName_SelectedIndexChanged);
			// 
			// comboBoxDescription
			// 
			this->comboBoxDescription->Visible = false;
			this->comboBoxDescription->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxDescription->FormattingEnabled = true;
			this->comboBoxDescription->Location = System::Drawing::Point(11, 96);
			this->comboBoxDescription->Name = L"comboBoxDescription";
			this->comboBoxDescription->Size = System::Drawing::Size(428, 21);
			this->comboBoxDescription->TabIndex = 5;
			this->comboBoxDescription->SelectedIndexChanged += gcnew System::EventHandler(this, &CamSelectionDialog::comboBoxDescription_SelectedIndexChanged);
			// 
			// labelFriendlyName
			// 
			this->labelFriendlyName->AutoSize = true;
			this->labelFriendlyName->Location = System::Drawing::Point(12, 9);
			this->labelFriendlyName->Name = L"labelFriendlyName";
			this->labelFriendlyName->Size = System::Drawing::Size(77, 13);
			this->labelFriendlyName->TabIndex = 6;
			this->labelFriendlyName->Text = L"Friendly Name:";
			// 
			// labelDescription
			// 
			this->labelDescription->Visible = false;
			this->labelDescription->AutoSize = true;
			this->labelDescription->Location = System::Drawing::Point(12, 70);
			this->labelDescription->Name = L"labelDescription";
			this->labelDescription->Size = System::Drawing::Size(63, 13);
			this->labelDescription->TabIndex = 7;
			this->labelDescription->Text = L"Description:";
			// 
			// toolTipDevicePath
			// 
			this->toolTipDevicePath->ToolTipIcon = System::Windows::Forms::ToolTipIcon::Info;
			this->toolTipDevicePath->ToolTipTitle = L"Camera Device Path";

			// 
			// CamSelectionDialog
			// 
			this->AcceptButton = this->buttonOK;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->buttonCancel;
			this->ClientSize = System::Drawing::Size(453, 273);
			this->Controls->Add(this->labelDescription);
			this->Controls->Add(this->labelFriendlyName);
			this->Controls->Add(this->comboBoxDescription);
			this->Controls->Add(this->comboBoxFriendlyName);
			this->Controls->Add(this->labelDevicePath);
			this->Controls->Add(this->textBoxDevicePath);
			this->Controls->Add(this->buttonCancel);
			this->Controls->Add(this->buttonOK);
			//this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"this.Icon")));
			this->Name = L"CamSelectionDialog";
			this->Text = L"CamSelectionDialog";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void comboBoxFriendlyName_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
			 {
				 if(this->comboBoxFriendlyName->SelectedIndex >= 0 && this->comboBoxDescription->Items->Count > 0 && (this->comboBoxDescription->SelectedIndex != this->comboBoxFriendlyName->SelectedIndex))
				 {
					 this->comboBoxDescription->SelectedIndex = this->comboBoxFriendlyName->SelectedIndex;
					 this->textBoxDevicePath->Text = this->_devicePath[this->comboBoxFriendlyName->SelectedIndex]->ToString();
				 }
			 }
	private: System::Void comboBoxDescription_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		 {
			 if(this->comboBoxDescription->SelectedIndex != this->comboBoxFriendlyName->SelectedIndex)
				 {
					 this->comboBoxFriendlyName->SelectedIndex = this->comboBoxDescription->SelectedIndex;
					 this->textBoxDevicePath->Text = this->_devicePath[this->comboBoxDescription->SelectedIndex]->ToString();
				 }
		 }
	private: System::Void buttonOK_Click(System::Object^  sender, System::EventArgs^  e)
		 {
			 this->_selectedOption = this->comboBoxFriendlyName->SelectedIndex;
			 this->Close();
		 }
private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e)
		 {
			 this->_selectedOption = -1;
			 this->Close();
		 }
};
}
