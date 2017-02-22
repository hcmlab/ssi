namespace mlpxml_csharp_test
{
    partial class form
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
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPageRun = new System.Windows.Forms.TabPage();
            this.textBoxAnno = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxSignal = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxTrainer = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.buttonStopRun = new System.Windows.Forms.Button();
            this.buttonStartRun = new System.Windows.Forms.Button();
            this.tabPageTrain = new System.Windows.Forms.TabPage();
            this.textBoxTraindef = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.buttonCancelTrain = new System.Windows.Forms.Button();
            this.buttonStartTrain = new System.Windows.Forms.Button();
            this.textBoxTrainerTrain = new System.Windows.Forms.TextBox();
            this.labelTrainer = new System.Windows.Forms.Label();
            this.textBoxTraining = new System.Windows.Forms.TextBox();
            this.labelTraining = new System.Windows.Forms.Label();
            this.labelPipeline = new System.Windows.Forms.Label();
            this.textBoxPipeline = new System.Windows.Forms.TextBox();
            this.labelXmlTrain = new System.Windows.Forms.Label();
            this.textBoxXmlTrain = new System.Windows.Forms.TextBox();
            this.textBoxConsole = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.checkBoxEvaluation = new System.Windows.Forms.CheckBox();
            this.comboBoxEval = new System.Windows.Forms.ComboBox();
            this.tabControl.SuspendLayout();
            this.tabPageRun.SuspendLayout();
            this.tabPageTrain.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Controls.Add(this.tabPageRun);
            this.tabControl.Controls.Add(this.tabPageTrain);
            this.tabControl.Location = new System.Drawing.Point(1, 77);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(464, 147);
            this.tabControl.TabIndex = 0;
            // 
            // tabPageRun
            // 
            this.tabPageRun.Controls.Add(this.textBoxAnno);
            this.tabPageRun.Controls.Add(this.label3);
            this.tabPageRun.Controls.Add(this.textBoxSignal);
            this.tabPageRun.Controls.Add(this.label2);
            this.tabPageRun.Controls.Add(this.textBoxTrainer);
            this.tabPageRun.Controls.Add(this.label1);
            this.tabPageRun.Controls.Add(this.buttonStopRun);
            this.tabPageRun.Controls.Add(this.buttonStartRun);
            this.tabPageRun.Location = new System.Drawing.Point(4, 22);
            this.tabPageRun.Name = "tabPageRun";
            this.tabPageRun.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageRun.Size = new System.Drawing.Size(456, 121);
            this.tabPageRun.TabIndex = 0;
            this.tabPageRun.Text = "Run";
            this.tabPageRun.UseVisualStyleBackColor = true;
            // 
            // textBoxAnno
            // 
            this.textBoxAnno.Location = new System.Drawing.Point(61, 34);
            this.textBoxAnno.Name = "textBoxAnno";
            this.textBoxAnno.Size = new System.Drawing.Size(384, 20);
            this.textBoxAnno.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(5, 37);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(35, 13);
            this.label3.TabIndex = 9;
            this.label3.Text = "Anno:";
            // 
            // textBoxSignal
            // 
            this.textBoxSignal.Location = new System.Drawing.Point(61, 10);
            this.textBoxSignal.Name = "textBoxSignal";
            this.textBoxSignal.Size = new System.Drawing.Size(384, 20);
            this.textBoxSignal.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(5, 13);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(39, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "Signal:";
            // 
            // textBoxTrainer
            // 
            this.textBoxTrainer.Location = new System.Drawing.Point(61, 60);
            this.textBoxTrainer.Name = "textBoxTrainer";
            this.textBoxTrainer.Size = new System.Drawing.Size(384, 20);
            this.textBoxTrainer.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(5, 63);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(43, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Trainer:";
            // 
            // buttonStopRun
            // 
            this.buttonStopRun.Location = new System.Drawing.Point(88, 88);
            this.buttonStopRun.Name = "buttonStopRun";
            this.buttonStopRun.Size = new System.Drawing.Size(75, 23);
            this.buttonStopRun.TabIndex = 6;
            this.buttonStopRun.Text = "Stop";
            this.buttonStopRun.UseVisualStyleBackColor = true;
            this.buttonStopRun.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // buttonStartRun
            // 
            this.buttonStartRun.Location = new System.Drawing.Point(7, 88);
            this.buttonStartRun.Name = "buttonStartRun";
            this.buttonStartRun.Size = new System.Drawing.Size(75, 23);
            this.buttonStartRun.TabIndex = 5;
            this.buttonStartRun.Text = "Start";
            this.buttonStartRun.UseVisualStyleBackColor = true;
            this.buttonStartRun.Click += new System.EventHandler(this.startButton_Click);
            // 
            // tabPageTrain
            // 
            this.tabPageTrain.Controls.Add(this.comboBoxEval);
            this.tabPageTrain.Controls.Add(this.checkBoxEvaluation);
            this.tabPageTrain.Controls.Add(this.textBoxTraindef);
            this.tabPageTrain.Controls.Add(this.label5);
            this.tabPageTrain.Controls.Add(this.buttonCancelTrain);
            this.tabPageTrain.Controls.Add(this.buttonStartTrain);
            this.tabPageTrain.Controls.Add(this.textBoxTrainerTrain);
            this.tabPageTrain.Controls.Add(this.labelTrainer);
            this.tabPageTrain.Controls.Add(this.textBoxTraining);
            this.tabPageTrain.Controls.Add(this.labelTraining);
            this.tabPageTrain.Location = new System.Drawing.Point(4, 22);
            this.tabPageTrain.Name = "tabPageTrain";
            this.tabPageTrain.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageTrain.Size = new System.Drawing.Size(456, 121);
            this.tabPageTrain.TabIndex = 1;
            this.tabPageTrain.Text = "Train";
            this.tabPageTrain.UseVisualStyleBackColor = true;
            // 
            // textBoxTraindef
            // 
            this.textBoxTraindef.Location = new System.Drawing.Point(63, 10);
            this.textBoxTraindef.Name = "textBoxTraindef";
            this.textBoxTraindef.Size = new System.Drawing.Size(384, 20);
            this.textBoxTraindef.TabIndex = 2;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(7, 13);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(49, 13);
            this.label5.TabIndex = 9;
            this.label5.Text = "Traindef:";
            // 
            // buttonCancelTrain
            // 
            this.buttonCancelTrain.Location = new System.Drawing.Point(88, 88);
            this.buttonCancelTrain.Name = "buttonCancelTrain";
            this.buttonCancelTrain.Size = new System.Drawing.Size(75, 23);
            this.buttonCancelTrain.TabIndex = 6;
            this.buttonCancelTrain.Text = "Cancel";
            this.buttonCancelTrain.UseVisualStyleBackColor = true;
            this.buttonCancelTrain.Click += new System.EventHandler(this.buttonTrainCancel_Click);
            // 
            // buttonStartTrain
            // 
            this.buttonStartTrain.Location = new System.Drawing.Point(7, 88);
            this.buttonStartTrain.Name = "buttonStartTrain";
            this.buttonStartTrain.Size = new System.Drawing.Size(75, 23);
            this.buttonStartTrain.TabIndex = 5;
            this.buttonStartTrain.Text = "Start";
            this.buttonStartTrain.UseVisualStyleBackColor = true;
            this.buttonStartTrain.Click += new System.EventHandler(this.buttonStartTrain_Click);
            // 
            // textBoxTrainerTrain
            // 
            this.textBoxTrainerTrain.Location = new System.Drawing.Point(111, 62);
            this.textBoxTrainerTrain.Name = "textBoxTrainerTrain";
            this.textBoxTrainerTrain.Size = new System.Drawing.Size(336, 20);
            this.textBoxTrainerTrain.TabIndex = 4;
            // 
            // labelTrainer
            // 
            this.labelTrainer.AutoSize = true;
            this.labelTrainer.Location = new System.Drawing.Point(7, 65);
            this.labelTrainer.Name = "labelTrainer";
            this.labelTrainer.Size = new System.Drawing.Size(98, 13);
            this.labelTrainer.TabIndex = 3;
            this.labelTrainer.Text = "Trainer/Evaluation:";
            // 
            // textBoxTraining
            // 
            this.textBoxTraining.Location = new System.Drawing.Point(63, 36);
            this.textBoxTraining.Name = "textBoxTraining";
            this.textBoxTraining.Size = new System.Drawing.Size(384, 20);
            this.textBoxTraining.TabIndex = 3;
            // 
            // labelTraining
            // 
            this.labelTraining.AutoSize = true;
            this.labelTraining.Location = new System.Drawing.Point(7, 39);
            this.labelTraining.Name = "labelTraining";
            this.labelTraining.Size = new System.Drawing.Size(48, 13);
            this.labelTraining.TabIndex = 3;
            this.labelTraining.Text = "Training:";
            // 
            // labelPipeline
            // 
            this.labelPipeline.AutoSize = true;
            this.labelPipeline.Location = new System.Drawing.Point(12, 39);
            this.labelPipeline.Name = "labelPipeline";
            this.labelPipeline.Size = new System.Drawing.Size(47, 13);
            this.labelPipeline.TabIndex = 3;
            this.labelPipeline.Text = "Pipeline:";
            // 
            // textBoxPipeline
            // 
            this.textBoxPipeline.Location = new System.Drawing.Point(68, 36);
            this.textBoxPipeline.Name = "textBoxPipeline";
            this.textBoxPipeline.Size = new System.Drawing.Size(384, 20);
            this.textBoxPipeline.TabIndex = 1;
            // 
            // labelXmlTrain
            // 
            this.labelXmlTrain.AutoSize = true;
            this.labelXmlTrain.Location = new System.Drawing.Point(12, 13);
            this.labelXmlTrain.Name = "labelXmlTrain";
            this.labelXmlTrain.Size = new System.Drawing.Size(51, 13);
            this.labelXmlTrain.TabIndex = 3;
            this.labelXmlTrain.Text = "XmlTrain:";
            // 
            // textBoxXmlTrain
            // 
            this.textBoxXmlTrain.Location = new System.Drawing.Point(68, 10);
            this.textBoxXmlTrain.Name = "textBoxXmlTrain";
            this.textBoxXmlTrain.Size = new System.Drawing.Size(384, 20);
            this.textBoxXmlTrain.TabIndex = 0;
            // 
            // textBoxConsole
            // 
            this.textBoxConsole.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxConsole.Location = new System.Drawing.Point(1, 243);
            this.textBoxConsole.Multiline = true;
            this.textBoxConsole.Name = "textBoxConsole";
            this.textBoxConsole.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxConsole.Size = new System.Drawing.Size(460, 146);
            this.textBoxConsole.TabIndex = 4;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 227);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(48, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "Console:";
            // 
            // checkBoxEvaluation
            // 
            this.checkBoxEvaluation.AutoSize = true;
            this.checkBoxEvaluation.Location = new System.Drawing.Point(232, 93);
            this.checkBoxEvaluation.Name = "checkBoxEvaluation";
            this.checkBoxEvaluation.Size = new System.Drawing.Size(76, 17);
            this.checkBoxEvaluation.TabIndex = 7;
            this.checkBoxEvaluation.Text = "Evaluation";
            this.checkBoxEvaluation.UseVisualStyleBackColor = true;
            // 
            // comboBoxEval
            // 
            this.comboBoxEval.DisplayMember = "0";
            this.comboBoxEval.FormattingEnabled = true;
            this.comboBoxEval.Items.AddRange(new object[] {
            "KFOLD",
            "LOO",
            "LOUO"});
            this.comboBoxEval.Location = new System.Drawing.Point(315, 91);
            this.comboBoxEval.Name = "comboBoxEval";
            this.comboBoxEval.Size = new System.Drawing.Size(132, 21);
            this.comboBoxEval.TabIndex = 8;
            // 
            // form
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(464, 389);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.textBoxConsole);
            this.Controls.Add(this.textBoxXmlTrain);
            this.Controls.Add(this.labelXmlTrain);
            this.Controls.Add(this.textBoxPipeline);
            this.Controls.Add(this.labelPipeline);
            this.Controls.Add(this.tabControl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "form";
            this.Text = "XmlTrainTest";
            this.tabControl.ResumeLayout(false);
            this.tabPageRun.ResumeLayout(false);
            this.tabPageRun.PerformLayout();
            this.tabPageTrain.ResumeLayout(false);
            this.tabPageTrain.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPageRun;
        private System.Windows.Forms.TabPage tabPageTrain;
        private System.Windows.Forms.Button buttonStopRun;
        private System.Windows.Forms.Button buttonStartRun;
        private System.Windows.Forms.Label labelPipeline;
        private System.Windows.Forms.TextBox textBoxPipeline;
        private System.Windows.Forms.Button buttonStartTrain;
        private System.Windows.Forms.TextBox textBoxTrainerTrain;
        private System.Windows.Forms.Label labelTrainer;
        private System.Windows.Forms.TextBox textBoxTraining;
        private System.Windows.Forms.Label labelTraining;
        private System.Windows.Forms.Label labelXmlTrain;
        private System.Windows.Forms.TextBox textBoxXmlTrain;
        private System.Windows.Forms.TextBox textBoxAnno;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxSignal;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxTrainer;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonCancelTrain;
        private System.Windows.Forms.TextBox textBoxConsole;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox textBoxTraindef;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox comboBoxEval;
        private System.Windows.Forms.CheckBox checkBoxEvaluation;
    }
}

