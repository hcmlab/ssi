#pragma once

//using namespace System;
//using namespace System::ComponentModel;
//using namespace System::Collections;
//using namespace System::Windows::Forms;
//using namespace System::Data;
//using namespace System::Drawing;

#include "DialogTemplate.h"
#include "MediaTypeInfo.h"
#include "GUIDStringToName.h"

namespace DialogLib {

	/// <summary>
	/// Summary for PinAndMediaSelectionDialog
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class PinAndMediaSelectionDialog : public DialogTemplate
	{
	public:
		PinAndMediaSelectionDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//

			_mediaTypeInfo = gcnew System::Collections::Generic::List<System::Collections::Generic::List<MediaTypeInfo^>^ >();
		}
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::RadioButton^  radioButtonFormatOther;
	public: 

	private: System::Windows::Forms::RadioButton^  radioButtonFormatDvInfo;

	private: System::Windows::Forms::RadioButton^  radioButtonFormatVideoInfo;
	private: System::Windows::Forms::CheckBox^  checkBoxInterleaveMode;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownWidthSelection;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownHeightSelection;
	private: System::Windows::Forms::Label^  labelWidthSelection;
	private: System::Windows::Forms::Label^  labelHeightSelection;
	private: System::Windows::Forms::Label^  labelDepthSelection;
	private: System::Windows::Forms::Label^  labelChannels;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownDepth;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownChannels;


	private: System::Windows::Forms::Label^  labelFPS;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownFPS;

	private: System::Windows::Forms::CheckBox^  checkBoxClosestFPS;
	private: System::Windows::Forms::CheckBox^  checkBoxFlipImage;
	private: System::Windows::Forms::TextBox^  textBoxSubType;
	private: System::Windows::Forms::Label^  labelSubType;




	public: System::Collections::Generic::List<System::Collections::Generic::List<MediaTypeInfo^>^ >^ _mediaTypeInfo;

	public: virtual int AddItem(System::String ^itemID, System::String ^itemValue) override;
	public: virtual bool AddItem(System::String ^itemID, int ordinalNumber, System::String ^itemValue) override;
	public: virtual bool RemoveItem(System::String ^itemID, int ordinalNumber) override;
	public: virtual bool RemoveItems(System::String ^itemID) override;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~PinAndMediaSelectionDialog()
		{
			if (components)
			{
				delete components;
			}
		}

	protected: 

	protected: 


	private: System::Windows::Forms::Button^  buttonOK;
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::TabControl^  tabControl1;

	private: System::Windows::Forms::TabPage^  tabPage2;
	private: System::Windows::Forms::ComboBox^  comboBoxPinIndex;
	private: System::Windows::Forms::Label^  labelPinIndex;
	private: System::Windows::Forms::Label^  labelMediaTypeIndex;
	private: System::Windows::Forms::ComboBox^  comboBoxMediaTypeIndex;
	private: System::Windows::Forms::PropertyGrid^  propertyGrid1;

	protected: 

	protected: 


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
			this->buttonOK = (gcnew System::Windows::Forms::Button());
			this->buttonCancel = (gcnew System::Windows::Forms::Button());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
			this->propertyGrid1 = (gcnew System::Windows::Forms::PropertyGrid());
			this->comboBoxPinIndex = (gcnew System::Windows::Forms::ComboBox());
			this->labelPinIndex = (gcnew System::Windows::Forms::Label());
			this->labelMediaTypeIndex = (gcnew System::Windows::Forms::Label());
			this->comboBoxMediaTypeIndex = (gcnew System::Windows::Forms::ComboBox());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->radioButtonFormatOther = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonFormatDvInfo = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonFormatVideoInfo = (gcnew System::Windows::Forms::RadioButton());
			this->checkBoxInterleaveMode = (gcnew System::Windows::Forms::CheckBox());
			this->numericUpDownWidthSelection = (gcnew System::Windows::Forms::NumericUpDown());
			this->numericUpDownHeightSelection = (gcnew System::Windows::Forms::NumericUpDown());
			this->labelWidthSelection = (gcnew System::Windows::Forms::Label());
			this->labelHeightSelection = (gcnew System::Windows::Forms::Label());
			this->labelDepthSelection = (gcnew System::Windows::Forms::Label());
			this->labelChannels = (gcnew System::Windows::Forms::Label());
			this->numericUpDownDepth = (gcnew System::Windows::Forms::NumericUpDown());
			this->numericUpDownChannels = (gcnew System::Windows::Forms::NumericUpDown());
			this->labelFPS = (gcnew System::Windows::Forms::Label());
			this->numericUpDownFPS = (gcnew System::Windows::Forms::NumericUpDown());
			this->checkBoxClosestFPS = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxFlipImage = (gcnew System::Windows::Forms::CheckBox());
			this->textBoxSubType = (gcnew System::Windows::Forms::TextBox());
			this->labelSubType = (gcnew System::Windows::Forms::Label());
			this->tabControl1->SuspendLayout();
			this->tabPage2->SuspendLayout();
			this->groupBox1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownWidthSelection))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownHeightSelection))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownDepth))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownChannels))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownFPS))->BeginInit();
			this->SuspendLayout();
			// 
			// buttonOK
			// 
			this->buttonOK->Location = System::Drawing::Point(656, 556);
			this->buttonOK->Name = L"buttonOK";
			this->buttonOK->Size = System::Drawing::Size(75, 23);
			this->buttonOK->TabIndex = 1;
			this->buttonOK->Text = L"OK";
			this->buttonOK->UseVisualStyleBackColor = true;
			this->buttonOK->Click += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::buttonOK_Click);
			// 
			// buttonCancel
			// 
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->buttonCancel->Location = System::Drawing::Point(737, 556);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(75, 23);
			this->buttonCancel->TabIndex = 2;
			this->buttonCancel->Text = L"Cancel";
			this->buttonCancel->UseVisualStyleBackColor = true;
			this->buttonCancel->Click += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::buttonCancel_Click);
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPage2);
			this->tabControl1->Location = System::Drawing::Point(12, 12);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(563, 574);
			this->tabControl1->TabIndex = 3;
			// 
			// tabPage2
			// 
			this->tabPage2->Controls->Add(this->propertyGrid1);
			this->tabPage2->Location = System::Drawing::Point(4, 22);
			this->tabPage2->Name = L"tabPage2";
			this->tabPage2->Padding = System::Windows::Forms::Padding(3);
			this->tabPage2->Size = System::Drawing::Size(555, 548);
			this->tabPage2->TabIndex = 1;
			this->tabPage2->Text = L"Type Info";
			this->tabPage2->UseVisualStyleBackColor = true;
			// 
			// propertyGrid1
			// 
			this->propertyGrid1->Location = System::Drawing::Point(7, 7);
			this->propertyGrid1->Name = L"propertyGrid1";
			this->propertyGrid1->Size = System::Drawing::Size(542, 535);
			this->propertyGrid1->TabIndex = 0;
			// 
			// comboBoxPinIndex
			// 
			this->comboBoxPinIndex->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxPinIndex->FormattingEnabled = true;
			this->comboBoxPinIndex->Location = System::Drawing::Point(581, 57);
			this->comboBoxPinIndex->Name = L"comboBoxPinIndex";
			this->comboBoxPinIndex->Size = System::Drawing::Size(111, 21);
			this->comboBoxPinIndex->TabIndex = 4;
			this->comboBoxPinIndex->SelectedIndexChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::comboBoxPinIndex_SelectedIndexChanged);
			// 
			// labelPinIndex
			// 
			this->labelPinIndex->AutoSize = true;
			this->labelPinIndex->Location = System::Drawing::Point(581, 34);
			this->labelPinIndex->Name = L"labelPinIndex";
			this->labelPinIndex->Size = System::Drawing::Size(111, 13);
			this->labelPinIndex->TabIndex = 5;
			this->labelPinIndex->Text = L"Select the Output Pin ";
			// 
			// labelMediaTypeIndex
			// 
			this->labelMediaTypeIndex->AutoSize = true;
			this->labelMediaTypeIndex->Location = System::Drawing::Point(698, 34);
			this->labelMediaTypeIndex->Name = L"labelMediaTypeIndex";
			this->labelMediaTypeIndex->Size = System::Drawing::Size(117, 13);
			this->labelMediaTypeIndex->TabIndex = 6;
			this->labelMediaTypeIndex->Text = L"Select the Media Type ";
			// 
			// comboBoxMediaTypeIndex
			// 
			this->comboBoxMediaTypeIndex->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxMediaTypeIndex->FormattingEnabled = true;
			this->comboBoxMediaTypeIndex->Location = System::Drawing::Point(701, 57);
			this->comboBoxMediaTypeIndex->Name = L"comboBoxMediaTypeIndex";
			this->comboBoxMediaTypeIndex->Size = System::Drawing::Size(111, 21);
			this->comboBoxMediaTypeIndex->TabIndex = 7;
			this->comboBoxMediaTypeIndex->SelectedIndexChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::comboBoxMediaTypeIndex_SelectedIndexChanged);
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->radioButtonFormatOther);
			this->groupBox1->Controls->Add(this->radioButtonFormatDvInfo);
			this->groupBox1->Controls->Add(this->radioButtonFormatVideoInfo);
			this->groupBox1->Location = System::Drawing::Point(581, 96);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(231, 94);
			this->groupBox1->TabIndex = 8;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Format Type";
			// 
			// radioButtonFormatOther
			// 
			this->radioButtonFormatOther->AutoCheck = false;
			this->radioButtonFormatOther->AutoSize = true;
			this->radioButtonFormatOther->Location = System::Drawing::Point(7, 66);
			this->radioButtonFormatOther->Name = L"radioButtonFormatOther";
			this->radioButtonFormatOther->Size = System::Drawing::Size(86, 17);
			this->radioButtonFormatOther->TabIndex = 2;
			this->radioButtonFormatOther->TabStop = true;
			this->radioButtonFormatOther->Text = L"Other Format";
			this->radioButtonFormatOther->UseVisualStyleBackColor = true;
			// 
			// radioButtonFormatDvInfo
			// 
			this->radioButtonFormatDvInfo->AutoCheck = false;
			this->radioButtonFormatDvInfo->AutoSize = true;
			this->radioButtonFormatDvInfo->Location = System::Drawing::Point(7, 43);
			this->radioButtonFormatDvInfo->Name = L"radioButtonFormatDvInfo";
			this->radioButtonFormatDvInfo->Size = System::Drawing::Size(108, 17);
			this->radioButtonFormatDvInfo->TabIndex = 1;
			this->radioButtonFormatDvInfo->TabStop = true;
			this->radioButtonFormatDvInfo->Text = L"FORMAT_DvInfo";
			this->radioButtonFormatDvInfo->UseVisualStyleBackColor = true;
			// 
			// radioButtonFormatVideoInfo
			// 
			this->radioButtonFormatVideoInfo->AutoCheck = false;
			this->radioButtonFormatVideoInfo->AutoSize = true;
			this->radioButtonFormatVideoInfo->Location = System::Drawing::Point(7, 20);
			this->radioButtonFormatVideoInfo->Name = L"radioButtonFormatVideoInfo";
			this->radioButtonFormatVideoInfo->Size = System::Drawing::Size(121, 17);
			this->radioButtonFormatVideoInfo->TabIndex = 0;
			this->radioButtonFormatVideoInfo->TabStop = true;
			this->radioButtonFormatVideoInfo->Text = L"FORMAT_VideoInfo";
			this->radioButtonFormatVideoInfo->UseVisualStyleBackColor = true;
			// 
			// checkBoxInterleaveMode
			// 
			this->checkBoxInterleaveMode->AutoCheck = false;
			this->checkBoxInterleaveMode->AutoSize = true;
			this->checkBoxInterleaveMode->Location = System::Drawing::Point(588, 196);
			this->checkBoxInterleaveMode->Name = L"checkBoxInterleaveMode";
			this->checkBoxInterleaveMode->Size = System::Drawing::Size(160, 17);
			this->checkBoxInterleaveMode->TabIndex = 9;
			this->checkBoxInterleaveMode->Text = L"Interleaved Mode necessary";
			this->checkBoxInterleaveMode->UseVisualStyleBackColor = true;
			this->checkBoxInterleaveMode->CheckedChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::checkBoxInterleaveMode_CheckedChanged);
			// 
			// numericUpDownWidthSelection
			// 
			this->numericUpDownWidthSelection->Location = System::Drawing::Point(588, 250);
			this->numericUpDownWidthSelection->Enabled = false;
			this->numericUpDownWidthSelection->Name = L"numericUpDownWidthSelection";
			this->numericUpDownWidthSelection->Size = System::Drawing::Size(104, 20);
			this->numericUpDownWidthSelection->TabIndex = 10;
			this->numericUpDownWidthSelection->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numericUpDownWidthSelection->ValueChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::numericUpDownWidthSelection_ValueChanged);
			// 
			// numericUpDownHeightSelection
			// 
			this->numericUpDownHeightSelection->Location = System::Drawing::Point(714, 250);
			this->numericUpDownHeightSelection->Enabled = false;
			this->numericUpDownHeightSelection->Name = L"numericUpDownHeightSelection";
			this->numericUpDownHeightSelection->Size = System::Drawing::Size(101, 20);
			this->numericUpDownHeightSelection->TabIndex = 11;
			this->numericUpDownHeightSelection->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numericUpDownHeightSelection->ValueChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::numericUpDownHeightSelection_ValueChanged);
			// 
			// labelWidthSelection
			// 
			this->labelWidthSelection->AutoSize = true;
			this->labelWidthSelection->Location = System::Drawing::Point(585, 225);
			this->labelWidthSelection->Name = L"labelWidthSelection";
			this->labelWidthSelection->Size = System::Drawing::Size(38, 13);
			this->labelWidthSelection->TabIndex = 12;
			this->labelWidthSelection->Text = L"Width ";
			// 
			// labelHeightSelection
			// 
			this->labelHeightSelection->AutoSize = true;
			this->labelHeightSelection->Location = System::Drawing::Point(711, 225);
			this->labelHeightSelection->Name = L"labelHeightSelection";
			this->labelHeightSelection->Size = System::Drawing::Size(41, 13);
			this->labelHeightSelection->TabIndex = 13;
			this->labelHeightSelection->Text = L"Height ";
			// 
			// labelDepthSelection
			// 
			this->labelDepthSelection->AutoSize = true;
			this->labelDepthSelection->Location = System::Drawing::Point(585, 284);
			this->labelDepthSelection->Name = L"labelDepthSelection";
			this->labelDepthSelection->Size = System::Drawing::Size(97, 13);
			this->labelDepthSelection->TabIndex = 14;
			this->labelDepthSelection->Text = L"Depth in Bits / Ch. ";
			// 
			// labelChannels
			// 
			this->labelChannels->AutoSize = true;
			this->labelChannels->Location = System::Drawing::Point(713, 284);
			this->labelChannels->Name = L"labelChannels";
			this->labelChannels->Size = System::Drawing::Size(64, 13);
			this->labelChannels->TabIndex = 15;
			this->labelChannels->Text = L"# Channels ";
			// 
			// numericUpDownDepth
			// 
			this->numericUpDownDepth->Enabled = false;
			this->numericUpDownDepth->Location = System::Drawing::Point(588, 311);
			this->numericUpDownDepth->Name = L"numericUpDownDepth";
			this->numericUpDownDepth->Size = System::Drawing::Size(104, 20);
			this->numericUpDownDepth->TabIndex = 16;
			this->numericUpDownDepth->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numericUpDownDepth->ValueChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::numericUpDownDepth_ValueChanged);
			// 
			// numericUpDownChannels
			// 
			this->numericUpDownChannels->Enabled = false;
			this->numericUpDownChannels->Location = System::Drawing::Point(714, 310);
			this->numericUpDownChannels->Name = L"numericUpDownChannels";
			this->numericUpDownChannels->Size = System::Drawing::Size(101, 20);
			this->numericUpDownChannels->TabIndex = 17;
			this->numericUpDownChannels->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numericUpDownChannels->ValueChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::numericUpDownChannels_ValueChanged);
			// 
			// labelFPS
			// 
			this->labelFPS->AutoSize = true;
			this->labelFPS->Location = System::Drawing::Point(585, 348);
			this->labelFPS->Name = L"labelFPS";
			this->labelFPS->Size = System::Drawing::Size(30, 13);
			this->labelFPS->TabIndex = 18;
			this->labelFPS->Text = L"FPS ";
			// 
			// numericUpDownFPS
			// 
			this->numericUpDownFPS->DecimalPlaces = 3;
			this->numericUpDownFPS->Location = System::Drawing::Point(588, 375);
			this->numericUpDownFPS->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1000, 0, 0, 0});
			this->numericUpDownFPS->Name = L"numericUpDownFPS";
			this->numericUpDownFPS->Size = System::Drawing::Size(104, 20);
			this->numericUpDownFPS->TabIndex = 19;
			this->numericUpDownFPS->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numericUpDownFPS->ValueChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::numericUpDownFPS_ValueChanged);
			// 
			// checkBoxClosestFPS
			// 
			this->checkBoxClosestFPS->AutoSize = true;
			this->checkBoxClosestFPS->Checked = true;
			this->checkBoxClosestFPS->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBoxClosestFPS->Location = System::Drawing::Point(714, 377);
			this->checkBoxClosestFPS->Name = L"checkBoxClosestFPS";
			this->checkBoxClosestFPS->Size = System::Drawing::Size(105, 17);
			this->checkBoxClosestFPS->TabIndex = 20;
			this->checkBoxClosestFPS->Text = L"Use Closest FPS";
			this->checkBoxClosestFPS->UseVisualStyleBackColor = true;
			this->checkBoxClosestFPS->CheckedChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::checkBoxClosestFPS_CheckedChanged);
			// 
			// checkBoxFlipImage
			// 
			this->checkBoxFlipImage->AutoSize = true;
			this->checkBoxFlipImage->Location = System::Drawing::Point(714, 411);
			this->checkBoxFlipImage->Name = L"checkBoxFlipImage";
			this->checkBoxFlipImage->Size = System::Drawing::Size(74, 17);
			this->checkBoxFlipImage->TabIndex = 21;
			this->checkBoxFlipImage->Text = L"Flip Image";
			this->checkBoxFlipImage->UseVisualStyleBackColor = true;
			this->checkBoxFlipImage->CheckedChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::checkBoxFlipImage_CheckedChanged);
			// 
			// textBoxSubType
			// 
			this->textBoxSubType->Location = System::Drawing::Point(588, 448);
			this->textBoxSubType->Name = L"textBoxSubType";
			this->textBoxSubType->ReadOnly = true;
			this->textBoxSubType->Size = System::Drawing::Size(224, 20);
			this->textBoxSubType->TabIndex = 22;
			this->textBoxSubType->TextChanged += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::textBoxSubType_TextChanged);
			// 
			// labelSubType
			// 
			this->labelSubType->AutoSize = true;
			this->labelSubType->Location = System::Drawing::Point(585, 415);
			this->labelSubType->Name = L"labelSubType";
			this->labelSubType->Size = System::Drawing::Size(53, 13);
			this->labelSubType->TabIndex = 23;
			this->labelSubType->Text = L"SubType ";
			// 
			// PinAndMediaSelectionDialog
			// 
			this->AcceptButton = this->buttonOK;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->buttonCancel;
			this->ClientSize = System::Drawing::Size(822, 595);
			this->Controls->Add(this->labelSubType);
			this->Controls->Add(this->textBoxSubType);
			this->Controls->Add(this->checkBoxFlipImage);
			this->Controls->Add(this->checkBoxClosestFPS);
			this->Controls->Add(this->numericUpDownFPS);
			this->Controls->Add(this->labelFPS);
			this->Controls->Add(this->numericUpDownChannels);
			this->Controls->Add(this->numericUpDownDepth);
			this->Controls->Add(this->labelChannels);
			this->Controls->Add(this->labelDepthSelection);
			this->Controls->Add(this->labelHeightSelection);
			this->Controls->Add(this->labelWidthSelection);
			this->Controls->Add(this->numericUpDownHeightSelection);
			this->Controls->Add(this->numericUpDownWidthSelection);
			this->Controls->Add(this->checkBoxInterleaveMode);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->comboBoxMediaTypeIndex);
			this->Controls->Add(this->labelMediaTypeIndex);
			this->Controls->Add(this->labelPinIndex);
			this->Controls->Add(this->comboBoxPinIndex);
			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->buttonCancel);
			this->Controls->Add(this->buttonOK);
			this->Name = L"PinAndMediaSelectionDialog";
			this->Text = L"PinAndMediaSelectionDialog";
			this->Load += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::PinAndMediaSelectionDialog_Load);
			this->Shown += gcnew System::EventHandler(this, &PinAndMediaSelectionDialog::PinAndMediaSelectionDialog_Shown);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &PinAndMediaSelectionDialog::PinAndMediaSelectionDialog_FormClosing);
			this->tabControl1->ResumeLayout(false);
			this->tabPage2->ResumeLayout(false);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownWidthSelection))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownHeightSelection))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownDepth))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownChannels))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDownFPS))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
private: System::Void buttonOK_Click(System::Object^  sender, System::EventArgs^  e) {
			this->_selectedOption = 0;
			this->Close();
		 }
private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e) {
			 this->_selectedOption = -1;
			 this->Close();
		 }
private: System::Void comboBoxPinIndex_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		 {
			 if(comboBoxPinIndex->SelectedIndex >= 0)
			 {
				System::Collections::Generic::List<MediaTypeInfo^>^ mediaTypeInfoListOfPin = _mediaTypeInfo[comboBoxPinIndex->SelectedIndex];
				comboBoxMediaTypeIndex->Items->Clear();
				for(int i = 0; i < mediaTypeInfoListOfPin->Count; ++i)
				{
					comboBoxMediaTypeIndex->Items->Add(i.ToString());
				}
			 }
			 else
			 {
				 comboBoxMediaTypeIndex->Items->Clear();
			 }
			 comboBoxMediaTypeIndex->SelectedIndex = -1;
			 propertyGrid1->SelectedObject = nullptr;
			 _returnInt["PinIndex"] = comboBoxPinIndex->SelectedIndex;
			 _returnInt["MediaTypeIndex"] = comboBoxMediaTypeIndex->SelectedIndex;
		 }
private: System::Void comboBoxMediaTypeIndex_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
		 {
			 if(comboBoxMediaTypeIndex->SelectedIndex >= 0)
			 {
				 System::Collections::Generic::List<MediaTypeInfo^>^ mediaTypeInfoListOfPin = _mediaTypeInfo[comboBoxPinIndex->SelectedIndex];
				 MediaTypeInfo^ currentMediaType = mediaTypeInfoListOfPin[comboBoxMediaTypeIndex->SelectedIndex];
				 propertyGrid1->SelectedObject = currentMediaType;

				 if(currentMediaType->formattype == "FORMAT_DvInfo")
				 {
					 radioButtonFormatVideoInfo->Checked = false;
					 radioButtonFormatDvInfo->Checked = true;
					 radioButtonFormatOther->Checked = false;

					 checkBoxInterleaveMode->Checked = true;

					 numericUpDownWidthSelection->Minimum = 720 / 4;
					 numericUpDownWidthSelection->Maximum = 720;
					 numericUpDownWidthSelection->Value = 720;
					 
					 numericUpDownHeightSelection->Minimum = (currentMediaType->dwDVVAuxSrc & 0x200000) ? (576 / 4) : (480 / 4);
					 numericUpDownHeightSelection->Maximum = (currentMediaType->dwDVVAuxSrc & 0x200000) ? (576) : (480);
					 numericUpDownHeightSelection->Value = (currentMediaType->dwDVVAuxSrc & 0x200000) ? (576) : (480);
					

					 //numericUpDownDepth->Minimum = 8;
					 //numericUpDownDepth->Maximum = 8;
					 numericUpDownDepth->Value = 8;

					 //numericUpDownChannels->Minimum = (currentMediaType->subtype == "MEDIASUBTYPE_RGB32") ? (4) : (3);
					 //numericUpDownChannels->Maximum = (currentMediaType->subtype == "MEDIASUBTYPE_RGB32") ? (4) : (3);
					 //numericUpDownChannels->Value = (currentMediaType->subtype == "MEDIASUBTYPE_RGB32") ? (4) : (3);
					 numericUpDownChannels->Value = 3;

					 //numericUpDownFPS->Minimum = (System::Decimal)(10000000.0 / (double)currentMediaType->MinFrameInterval);
					 //numericUpDownFPS->Maximum = (System::Decimal)(10000000.0 / (double)currentMediaType->MaxFrameInterval);
					 numericUpDownFPS->Value = (System::Decimal)((currentMediaType->dwDVVAuxSrc & 0x200000) ? (25.0) : (29.97));
					 textBoxSubType->Text = currentMediaType->subtype;

				 }
				 else if(currentMediaType->formattype == "FORMAT_VideoInfo")
				 {
					 radioButtonFormatVideoInfo->Checked = true;
					 radioButtonFormatDvInfo->Checked = false;
					 radioButtonFormatOther->Checked = false;

					 checkBoxInterleaveMode->Checked = false;

					 numericUpDownWidthSelection->Minimum = currentMediaType->MinOutputSize.Width;
					 numericUpDownWidthSelection->Maximum = currentMediaType->MaxOutputSize.Width;
					 numericUpDownWidthSelection->Value = currentMediaType->MinOutputSize.Width;
					 
					 numericUpDownHeightSelection->Minimum = currentMediaType->MinOutputSize.Height;
					 numericUpDownHeightSelection->Maximum = currentMediaType->MaxOutputSize.Height;
					 numericUpDownHeightSelection->Value = currentMediaType->MinOutputSize.Height;
					

					 /*numericUpDownDepth->Minimum = 8;
					 numericUpDownDepth->Maximum = 8;*/
					 numericUpDownDepth->Value = 8;

					 /*
					 numericUpDownChannels->Minimum = (currentMediaType->subtype == "MEDIASUBTYPE_RGB32") ? (4) : (3);
					 numericUpDownChannels->Maximum = (currentMediaType->subtype == "MEDIASUBTYPE_RGB32") ? (4) : (3);
					 numericUpDownChannels->Value = (currentMediaType->subtype == "MEDIASUBTYPE_RGB32") ? (4) : (3);
					 //*/
					 numericUpDownChannels->Value = 3;

					 //numericUpDownFPS->Minimum = (System::Decimal)(10000000.0 / (double)currentMediaType->MinFrameInterval);
					 //numericUpDownFPS->Maximum = (System::Decimal)(10000000.0 / (double)currentMediaType->MaxFrameInterval);
					 numericUpDownFPS->Value = (System::Decimal)(10000000.0 / (double)currentMediaType->MaxFrameInterval);

					 textBoxSubType->Text = currentMediaType->subtype;

				 }
				 else
				 {
					 radioButtonFormatVideoInfo->Checked = false;
					 radioButtonFormatDvInfo->Checked = false;
					 radioButtonFormatOther->Checked = true;

					 checkBoxInterleaveMode->Checked = false;

					 numericUpDownWidthSelection->Minimum = 0;
					 numericUpDownWidthSelection->Maximum = 10000;
					 
					 
					 numericUpDownHeightSelection->Minimum = 0;
					 numericUpDownHeightSelection->Maximum = 10000;
	

					 numericUpDownWidthSelection->Value = 0;
					 numericUpDownHeightSelection->Value = 0;
					 numericUpDownDepth->Value = 0;
					 numericUpDownChannels->Value = 0;
					 numericUpDownFPS->Value = 0;

					 textBoxSubType->Text = System::String::Empty;

				 }


			 }
			 else
			 {
				 propertyGrid1->SelectedObject = nullptr;

				 radioButtonFormatVideoInfo->Checked = false;
				 radioButtonFormatDvInfo->Checked = false;
				 radioButtonFormatOther->Checked = false;

				 checkBoxInterleaveMode->Checked = false;

				 numericUpDownWidthSelection->Value = 0;
				 numericUpDownHeightSelection->Value = 0;
				 numericUpDownDepth->Value = 0;
				 numericUpDownChannels->Value = 0;
				 numericUpDownFPS->Value = 0;

				 textBoxSubType->Text = System::String::Empty;
			 }
			 _returnInt["PinIndex"] = comboBoxPinIndex->SelectedIndex;
			 _returnInt["MediaTypeIndex"] = comboBoxMediaTypeIndex->SelectedIndex;

			 _returnInt["WidthInPixels"] = (int)numericUpDownWidthSelection->Value;
			 _returnInt["HeightInPixels"] = (int)numericUpDownHeightSelection->Value;
			 _returnInt["DepthInBitsPerChannel"] = (int)numericUpDownDepth->Value;
			 _returnInt["NumOfChannels"] = (int)numericUpDownChannels->Value;
			 _returnDouble["FramesPerSecond"] = (double)numericUpDownFPS->Value;
			 _returnInt["UseClosestFramerateForGraph"] = (checkBoxClosestFPS->Checked) ? (1) : (0);
			 _returnInt["FlipImage"] = (checkBoxFlipImage->Checked) ? (1) : (0);
			 _returnInt["MajorVideoType"] = (checkBoxInterleaveMode->Checked) ? (2) : (1);
			 if(!GUIDStringToName::_isInit)
			 {
				 GUIDStringToName::InitGUIDMap();
			 }
			 _returnString["OutputSubTypeOfCaptureDevice"] = (GUIDStringToName::StringMap->ContainsKey(textBoxSubType->Text)) ?
																	(GUIDStringToName::StringMap[textBoxSubType->Text]) :
																	(gcnew System::String(textBoxSubType->Text));
		 }
private: System::Void PinAndMediaSelectionDialog_Load(System::Object^  sender, System::EventArgs^  e) {

			 this->_selectedOption = -1;
			 radioButtonFormatVideoInfo->Checked = false;
			radioButtonFormatDvInfo->Checked = false;
			radioButtonFormatOther->Checked = true;

			checkBoxInterleaveMode->Checked = false;

			numericUpDownWidthSelection->Value = 0;
			numericUpDownHeightSelection->Value = 0;
			numericUpDownDepth->Value = 8;
			numericUpDownChannels->Value = 3;
			numericUpDownFPS->Value = 0;

			textBoxSubType->Text = System::String::Empty;

			_returnInt["PinIndex"] = comboBoxPinIndex->SelectedIndex;
			_returnInt["MediaTypeIndex"] = comboBoxMediaTypeIndex->SelectedIndex;

			_returnInt["WidthInPixels"] = (int)numericUpDownWidthSelection->Value;
			_returnInt["HeightInPixels"] = (int)numericUpDownHeightSelection->Value;
			_returnInt["DepthInBitsPerChannel"] = (int)numericUpDownDepth->Value;
			_returnInt["NumOfChannels"] = (int)numericUpDownChannels->Value;
			_returnDouble["FramesPerSecond"] = (double)numericUpDownFPS->Value;
			_returnInt["UseClosestFramerateForGraph"] = (checkBoxClosestFPS->Checked) ? (1) : (0);
			_returnInt["FlipImage"] = (checkBoxFlipImage->Checked) ? (1) : (0);
			_returnInt["MajorVideoType"] = (checkBoxInterleaveMode->Checked) ? (2) : (1);
			if(!GUIDStringToName::_isInit)
			{
			 GUIDStringToName::InitGUIDMap();
			}
			_returnString["OutputSubTypeOfCaptureDevice"] = (GUIDStringToName::StringMap->ContainsKey(textBoxSubType->Text)) ?
																(GUIDStringToName::StringMap[textBoxSubType->Text]) :
																(gcnew System::String(textBoxSubType->Text));

		 }
private: System::Void PinAndMediaSelectionDialog_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			 int pinIndex = comboBoxPinIndex->SelectedIndex;
			 int mediaTypeIndex = comboBoxMediaTypeIndex->SelectedIndex;
			 if(pinIndex >= 0 && mediaTypeIndex >= 0)
			 {
				 System::Collections::Generic::List<MediaTypeInfo^>^ mediaTypeInfoListOfPin = _mediaTypeInfo[comboBoxPinIndex->SelectedIndex];
				 MediaTypeInfo^ currentMediaType = mediaTypeInfoListOfPin[comboBoxMediaTypeIndex->SelectedIndex];
				 propertyGrid1->SelectedObject = currentMediaType;

				 if(currentMediaType->subtype == "MEDIASUBTYPE_dvsd")
				 {
					 int outputX = 720;
					 int outputY = (currentMediaType->dwDVVAuxSrc & 0x200000) ? (576) : (480);
					 double outputFPS = (currentMediaType->dwDVVAuxSrc & 0x200000) ? (25.0) : (29.97);
					 if(outputFPS ==_returnDouble["FramesPerSecond"])
					 {
						 //OK
					 }
					 else if(_returnDouble["FramesPerSecond"] > 0.0 && checkBoxClosestFPS->Checked)
					 {
						 //OK
					 }
					 else
					 {
						 if(System::Windows::Forms::MessageBox::Show("Framerate not supportet.\nEither turn on to take the closest possible framerate and then stream with the desired framerate, or choose a valid framerate!\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
							 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
							 System::Windows::Forms::DialogResult::Cancel)
						 {
							 this->_selectedOption = -1;
							 e->Cancel = true;
						 }
						 else
						 {
							 this->_selectedOption = -1;
						 }
					 }
					 if(( (_returnInt["WidthInPixels"] == outputX) && (_returnInt["HeightInPixels"] == outputY)) ||
						 ( (_returnInt["WidthInPixels"] == (outputX / 2)) && (_returnInt["HeightInPixels"] == (outputY / 2))) ||
						 ( (_returnInt["WidthInPixels"] == (outputX / 4)) && (_returnInt["HeightInPixels"] == (outputY / 4))))
					 {
						 //OK
					 }
					 else
					 {
						 if(System::Windows::Forms::MessageBox::Show("Resolution not supportet.\nFor DV only native PAL/NTSC resolution or the half or quarter of it is supportet.\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
							 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
							 System::Windows::Forms::DialogResult::Cancel)
						 {
							 this->_selectedOption = -1;
							 e->Cancel = true;
						 }
						 else
						 {
							 this->_selectedOption = -1;
						 }
					 }
				 }
				 else if((/*currentMediaType->subtype == "MEDIASUBTYPE_RGB24" &&*/ currentMediaType->formattype == "FORMAT_VideoInfo") ||
					 (/*currentMediaType->subtype == "MEDIASUBTYPE_RGB32" &&*/ currentMediaType->formattype == "FORMAT_VideoInfo"))
				 {
					 System::Int64 fps = (System::Int64)(10000000.0 / _returnDouble["FramesPerSecond"] + 0.5);
					 if((fps >= currentMediaType->MinFrameInterval) && (fps <= currentMediaType->MaxFrameInterval) )
					 {
						 //OK
					 }
					 else if(checkBoxClosestFPS->Checked)
					 {
						 //OK
					 }
					 else
					 {
						 if(System::Windows::Forms::MessageBox::Show("Framerate not supportet.\nEither turn on to take the closest possible framerate and then stream with the desired framerate, or choose a valid framerate!\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
							 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
							 System::Windows::Forms::DialogResult::Cancel)
						 {
							 this->_selectedOption = -1;
							 e->Cancel = true;
						 }
						 else
						 {
							 this->_selectedOption = -1;
						 }
					 }
					 bool foundHeight = false;
					 bool foundWidth = false;
					 if(System::Math::Abs(currentMediaType->biHeight) != _returnInt["HeightInPixels"])
					 {
						 if(currentMediaType->MinOutputSize.Height <= _returnInt["HeightInPixels"] && currentMediaType->MaxOutputSize.Height >= _returnInt["HeightInPixels"])
							{
								for(int yRunner = currentMediaType->MinOutputSize.Height; yRunner < currentMediaType->MaxOutputSize.Height; yRunner += currentMediaType->OutputGranularityY)
								{
									if(yRunner == _returnInt["HeightInPixels"])
									{
										foundHeight = true;
										//FOUND height format
									}//END yRunner == height
								}//END FOR yRunner in range
							}////END IF desired Format in range
					 }
					 else
					 {
						 //OK
						 foundHeight = true;
					 }
					 if(System::Math::Abs(currentMediaType->biWidth) != _returnInt["WidthInPixels"])
					 {
						 if(currentMediaType->MinOutputSize.Width <= _returnInt["WidthInPixels"] && currentMediaType->MaxOutputSize.Width >= _returnInt["WidthInPixels"])
							{
								for(int xRunner = currentMediaType->MinOutputSize.Width; xRunner < currentMediaType->MaxOutputSize.Width; xRunner += currentMediaType->OutputGranularityX)
								{
									if(xRunner == _returnInt["WidthInPixels"])
									{
										foundWidth = true;
										//FOUND height Width
									}//END xRunner == Width
								}//END FOR xRunner in range
							}////END IF desired Format in range
					 }
					 else
					 {
						 //OK
						 foundWidth = true;
					 }
					 if(foundWidth && foundHeight)
					 {
						 //OK
					 }
					 else
					 {
						 if(System::Windows::Forms::MessageBox::Show("Resolution not supportet.\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
							 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
							 System::Windows::Forms::DialogResult::Cancel)
						 {
							 this->_selectedOption = -1;
							 e->Cancel = true;
						 }
						 else
						 {
							 //this->_selectedOption = -1;
						 }
					 }
					 if(_returnInt["DepthInBitsPerChannel"] != 8)
					 {
						 if(System::Windows::Forms::MessageBox::Show("DepthInBitsPerChannel not supportet or only experimental support.\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
							 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
							 System::Windows::Forms::DialogResult::Cancel)
						 {
							 this->_selectedOption = -1;
							 e->Cancel = true;
						 }
						 else
						 {
							 //this->_selectedOption = -1;
						 }
					 }
					 if(!((_returnInt["NumOfChannels"] == 3) ||
						 (_returnInt["NumOfChannels"] == 4)))
					 {
						 if(System::Windows::Forms::MessageBox::Show("NumOfChannels not supportet or only experimental support.\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
							 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
							 System::Windows::Forms::DialogResult::Cancel)
						 {
							 this->_selectedOption = -1;
							 e->Cancel = true;
						 }
						 else
						 {
							 //this->_selectedOption = -1;
						 }
					 }
				 }
				 else
				 {
					 if(System::Windows::Forms::MessageBox::Show("Subtype not supportet or only experimental support.\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
						 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
						 System::Windows::Forms::DialogResult::Cancel)
					 {
						 this->_selectedOption = -1;
						 e->Cancel = true;
					 }
					 else
					 {
						 //this->_selectedOption = -1;
					 }

				 }
			 }
			 else
			 {
				 if(System::Windows::Forms::MessageBox::Show("No MediaType selected.\nPress Cancel to select again!", "Attention!", System::Windows::Forms::MessageBoxButtons::OKCancel,
					 System::Windows::Forms::MessageBoxIcon::Hand, System::Windows::Forms::MessageBoxDefaultButton::Button2) ==
					 System::Windows::Forms::DialogResult::Cancel)
				 {
					 this->_selectedOption = -1;
					 e->Cancel = true;
				 }
				 else
				 {
					 this->_selectedOption = -1;
				 }
			 }
		 }
private: System::Void numericUpDownFPS_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 _returnDouble["FramesPerSecond"] = (double)numericUpDownFPS->Value;
		 }
private: System::Void numericUpDownWidthSelection_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 _returnInt["WidthInPixels"] = (int)numericUpDownWidthSelection->Value;
		 }
private: System::Void numericUpDownHeightSelection_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 _returnInt["HeightInPixels"] = (int)numericUpDownHeightSelection->Value;
		 }
private: System::Void numericUpDownDepth_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			  //_returnInt["DepthInBitsPerChannel"] = (int)numericUpDownDepth->Value;
			  _returnInt["DepthInBitsPerChannel"] = 8;
		 }
private: System::Void numericUpDownChannels_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 //_returnInt["NumOfChannels"] = (int)numericUpDownChannels->Value;
			 _returnInt["NumOfChannels"] = 3;
		 }
private: System::Void checkBoxClosestFPS_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 _returnInt["UseClosestFramerateForGraph"] = (checkBoxClosestFPS->Checked) ? (1) : (0);
		 }
private: System::Void checkBoxFlipImage_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 _returnInt["FlipImage"] = (checkBoxFlipImage->Checked) ? (1) : (0);
		 }
private: System::Void checkBoxInterleaveMode_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 _returnInt["MajorVideoType"] = (checkBoxInterleaveMode->Checked) ? (2) : (1);
		 }
private: System::Void textBoxSubType_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 if(!GUIDStringToName::_isInit)
			{
			 GUIDStringToName::InitGUIDMap();
			}
			_returnString["OutputSubTypeOfCaptureDevice"] = (GUIDStringToName::StringMap->ContainsKey(textBoxSubType->Text)) ?
																(GUIDStringToName::StringMap[textBoxSubType->Text]) :
																(gcnew System::String(textBoxSubType->Text));

		 }
private: System::Void PinAndMediaSelectionDialog_Shown(System::Object^  sender, System::EventArgs^  e) {
			comboBoxPinIndex->SelectedIndex = 0;
			comboBoxMediaTypeIndex->SelectedIndex = 0;	
		 }
};
}
