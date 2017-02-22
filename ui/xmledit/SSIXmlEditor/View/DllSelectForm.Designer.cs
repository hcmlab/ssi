namespace SSIXmlEditor
{
    partial class DllSelectForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DllSelectForm));
            this.AvailableListBox = new System.Windows.Forms.ListBox();
            this.SelectButton = new System.Windows.Forms.Button();
            this.DeselectButton = new System.Windows.Forms.Button();
            this.SelectedListBox = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.StartButton = new System.Windows.Forms.Button();
            this.CancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // AvailableListBox
            // 
            this.AvailableListBox.FormattingEnabled = true;
            this.AvailableListBox.Location = new System.Drawing.Point(12, 36);
            this.AvailableListBox.Name = "AvailableListBox";
            this.AvailableListBox.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.AvailableListBox.Size = new System.Drawing.Size(270, 173);
            this.AvailableListBox.TabIndex = 0;
            // 
            // SelectButton
            // 
            this.SelectButton.Location = new System.Drawing.Point(288, 38);
            this.SelectButton.Name = "SelectButton";
            this.SelectButton.Size = new System.Drawing.Size(21, 23);
            this.SelectButton.TabIndex = 1;
            this.SelectButton.Text = ">";
            this.SelectButton.UseVisualStyleBackColor = true;
            this.SelectButton.Click += new System.EventHandler(this.SelectButton_Click);
            // 
            // DeselectButton
            // 
            this.DeselectButton.Location = new System.Drawing.Point(288, 67);
            this.DeselectButton.Name = "DeselectButton";
            this.DeselectButton.Size = new System.Drawing.Size(21, 23);
            this.DeselectButton.TabIndex = 2;
            this.DeselectButton.Text = "<";
            this.DeselectButton.UseVisualStyleBackColor = true;
            this.DeselectButton.Click += new System.EventHandler(this.DeselectButton_Click);
            // 
            // SelectedListBox
            // 
            this.SelectedListBox.FormattingEnabled = true;
            this.SelectedListBox.Location = new System.Drawing.Point(316, 36);
            this.SelectedListBox.Name = "SelectedListBox";
            this.SelectedListBox.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.SelectedListBox.Size = new System.Drawing.Size(270, 173);
            this.SelectedListBox.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Available:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(313, 13);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(52, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Selected:";
            // 
            // StartButton
            // 
            this.StartButton.Location = new System.Drawing.Point(430, 217);
            this.StartButton.Name = "StartButton";
            this.StartButton.Size = new System.Drawing.Size(75, 23);
            this.StartButton.TabIndex = 7;
            this.StartButton.Text = "Start";
            this.StartButton.UseVisualStyleBackColor = true;
            this.StartButton.Click += new System.EventHandler(this.StartButton_Click);
            // 
            // CancelButton
            // 
            this.CancelButton.Location = new System.Drawing.Point(511, 217);
            this.CancelButton.Name = "CancelButton";
            this.CancelButton.Size = new System.Drawing.Size(75, 23);
            this.CancelButton.TabIndex = 8;
            this.CancelButton.Text = "Cancel";
            this.CancelButton.UseVisualStyleBackColor = true;
            this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
            // 
            // DllSelectForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(598, 252);
            this.Controls.Add(this.CancelButton);
            this.Controls.Add(this.StartButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.SelectedListBox);
            this.Controls.Add(this.DeselectButton);
            this.Controls.Add(this.SelectButton);
            this.Controls.Add(this.AvailableListBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DllSelectForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Select DLLs";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox AvailableListBox;
        private System.Windows.Forms.Button SelectButton;
        private System.Windows.Forms.Button DeselectButton;
        private System.Windows.Forms.ListBox SelectedListBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button StartButton;
        private System.Windows.Forms.Button CancelButton;
    }
}