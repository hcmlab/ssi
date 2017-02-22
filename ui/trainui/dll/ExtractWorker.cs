using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;

namespace ssi
{
    public class ExtractWorker : BackgroundWorker
    {        
        public ExtractWorker()            
        {            
            this.WorkerReportsProgress = true;
            this.WorkerSupportsCancellation = true;
            this.DoWork += OnDoWork;            
        }

        void OnDoWork(object s, DoWorkEventArgs args)
        {
            BackgroundWorker w = s as BackgroundWorker;
            TrainHandler h = args.Argument as TrainHandler;

            Stopwatch sw = new Stopwatch();
            sw.Start();

            if (h.Project.MLPCollectInit(h.Info.Model, h.Info.Annotation.Name))
            {
                foreach (SelectItem dir in h.Info.Dates)
                {
                    w.ReportProgress(0, "Extracting features for " + dir.Path);

                    h.Project.MLPCollect(dir.Path, h.Info.ReExtract);

                    if (w.CancellationPending)
                    {
                        args.Cancel = true;
                        return;
                    }
                    w.ReportProgress(1);
                }
            }

            if (h.Info.EvalOn)
            {                  
                w.ReportProgress(0, "Running evaluation..");
                h.Project.MLPEval(h.Info.Model, TrainHandler.EVAL_TMP_FILEPATH, h.Info.EvalType, h.Info.EvalKFolds);
                if (File.Exists(TrainHandler.EVAL_TMP_FILEPATH))
                {
                    string evalText = File.ReadAllText(TrainHandler.EVAL_TMP_FILEPATH);
                    args.Result = evalText;
                    File.Delete(TrainHandler.EVAL_TMP_FILEPATH);
                }
            }
            else
            {
                w.ReportProgress(0, "Running training..");
                string now = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");
                string filedir = h.Project.ModelDir + "\\" + now;
                Directory.CreateDirectory(filedir);
                if (h.Project.MLPTrain(h.Info.Model, filedir))
                {
                    //h.Info.Save(filedir);                    
                    h.Project.MLPCollectSave(filedir);
                }
                args.Result = filedir;
            }

            if (w.CancellationPending)
            {
                args.Cancel = true;
                return;
            }

            sw.Stop();
            w.ReportProgress(1, "Finished (elapsed time " + sw.Elapsed.ToString() + ")");                        
        }

        public void Do(TrainHandler handler)
        {           
            this.RunWorkerAsync(handler);
        }
    }
}
