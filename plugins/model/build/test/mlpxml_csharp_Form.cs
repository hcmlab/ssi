// mlpxml_csharp_Form.h
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2011/15/27
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading;

namespace mlpxml_csharp_test
{
    public partial class form : Form
    {
        delegate void SetRunStateSafe (bool state);
        delegate void SetTrainStateSafe(bool state);
        delegate void WriteOnConsoleSafe(string line);        

        public form()
        {
            InitializeComponent();

#if DEBUG           
            textBoxXmlTrain.Text = @"..\..\..\..\..\..\..\bin\Win32\vc100\mlpxmld.exe";
#else
            textBoxXmlTrain.Text = @"..\..\..\..\..\..\..\bin\Win32\vc100\mlpxml.exe";            
#endif
            textBoxPipeline.Text = @"..\..\..\mlp";
            textBoxTrainerTrain.Text = @"mlp";
            textBoxTraining.Text = @"mlp";
            textBoxSignal.Text = @"mlp";
            textBoxAnno.Text = @"mlp";
            textBoxTrainer.Text = @"mlp";
            textBoxTraindef.Text = @"mlp";
            
            buttonStopRun.Enabled = false;
            buttonCancelTrain.Enabled = false;
            comboBoxEval.SelectedIndex = 0;
            
            ssi.MlpXmlRun.processStartEvent += processRunStart;
            ssi.MlpXmlRun.processStopEvent += processRunStop;
            ssi.MlpXmlRun.processOutputEvent += processOutputEvent;
            ssi.MlpXmlRun.processUpdateEvent += processUpdateEvent;
            ssi.MlpXmlTrain.processStartEvent += processTrainStart;
            ssi.MlpXmlTrain.processStopEvent += processTrainStop;
            ssi.MlpXmlTrain.processOutputEvent += processOutputEvent;            
        }        

        private void writeOnConsole (string line)
        {
            textBoxConsole.Text += line;
            textBoxConsole.Text += Environment.NewLine;
            textBoxConsole.SelectionStart = textBoxConsole.Text.Length;
            textBoxConsole.ScrollToCaret();
        }

        void processUpdateEvent(double start, double duration, string name)
        {
            string line = start + "," + duration + "," + name;
            processOutputEvent(line);
        }

        void processOutputEvent(string line)
        {
            WriteOnConsoleSafe d = new WriteOnConsoleSafe(writeOnConsole);
            this.Invoke(d, new object[] { line });
        }

        private void setRunState(bool state)
        {
            buttonStartRun.Enabled = !state;
            buttonStopRun.Enabled = state;
            form.ActiveForm.ControlBox = !state;
        }

        private void processRunStart()
        {
            SetRunStateSafe d = new SetRunStateSafe(setRunState);
            this.Invoke(d, new object[] { true });
        }

        private void processRunStop()
        {
            SetRunStateSafe d = new SetRunStateSafe(setRunState);
            this.Invoke(d, new object[] { false });
        }

        private void setTrainState(bool state)
        {
            buttonStartTrain.Enabled = !state;
            buttonCancelTrain.Enabled = state;
            form.ActiveForm.ControlBox = !state;
        }

        private void processTrainStart()
        {
            SetRunStateSafe d = new SetRunStateSafe(setTrainState);
            this.Invoke(d, new object[] { true });
        }

        private void processTrainStop()
        {
            SetRunStateSafe d = new SetRunStateSafe(setTrainState);
            this.Invoke(d, new object[] { false });
        }

        private void startButton_Click(object sender, EventArgs e)
        {
            textBoxConsole.Text = "";
            buttonStartRun.Enabled = false;                 
            ssi.MlpXmlRun.trainer = textBoxTrainer.Text;
            ssi.MlpXmlRun.signal = textBoxSignal.Text;
            ssi.MlpXmlRun.anno = textBoxAnno.Text;
            ssi.MlpXmlRun.xmlTrainFilepath = textBoxXmlTrain.Text;
            ssi.MlpXmlRun.pipelineFilepath = textBoxPipeline.Text;
            ThreadStart threadStart = new ThreadStart(ssi.MlpXmlRun.Start);
            Thread thread = new Thread(threadStart);
            thread.Start();            
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            ssi.MlpXmlRun.Stop();
            buttonStopRun.Enabled = false;
        }

        private void buttonStartTrain_Click(object sender, EventArgs e)
        {
            textBoxConsole.Text = "";
            ssi.MlpXmlTrain.eval = checkBoxEvaluation.Checked ? comboBoxEval.SelectedIndex : -1;       
            ssi.MlpXmlTrain.traindef = textBoxTraindef.Text;
            ssi.MlpXmlTrain.trainer = textBoxTrainerTrain.Text;
            ssi.MlpXmlTrain.training = textBoxTraining.Text;
            ssi.MlpXmlTrain.xmlTrainFilepath = textBoxXmlTrain.Text;
            ssi.MlpXmlTrain.pipelineFilepath = textBoxPipeline.Text;
            ThreadStart threadStart = new ThreadStart(ssi.MlpXmlTrain.Start);
            Thread thread = new Thread(threadStart);
            thread.Start();
        }

        private void buttonTrainCancel_Click(object sender, EventArgs e)
        {
            ssi.MlpXmlTrain.Cancel();
        }
    }

    
}
