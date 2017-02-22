using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FusionUI
{
    public enum DecayFunctionType
    {
        NoDecay = 0,
        Linear = 1,
        Hyperbolic = 2,
        Exponential = 3
    }

    public class Entity
    {
        public Entity(EntityContainer _owningContainer)
        {
            idName = string.Empty;
            decayFunction = DecayFunctionType.Hyperbolic;
            //timeLastUpdated = 0.0f;
            weightFactor = 0.5f;
            timeTillVectorBase = 20.0f;
            timeTillVectorBaseGUI = TimeSpan.FromSeconds(timeTillVectorBase);
            timeLastUpdatedGUI = DateTime.MinValue;
            lambda = 0.5f;
            vectorBase = 0.0f;
            axisVal[0] = 0.0f;
            axisVal[1] = 0.0f;
            axisVal[2] = 0.0f;
            owningContainer = _owningContainer;
        }

        private string idName;
        private DecayFunctionType decayFunction;
        //private float timeLastUpdated;
        private DateTime timeLastUpdatedGUI;
        private float weightFactor;
        private float timeTillVectorBase;
        private TimeSpan timeTillVectorBaseGUI;
        private float lambda;
        private float vectorBase;
        private float currentDecayFactor;
        private float[] axisVal = new float[3];
        private float tmpXCoordinate = 0.0f;
        private float tmpYCoordinate = 0.0f;
        private bool isEntityActive = true;

        [System.ComponentModel.Description("Determines if the Entity is drawn and if it's values are considered when calculating the combined vector and mass center. Setting this to false has the same effect as setting weight to 0."),
        System.ComponentModel.Category("Props of Selected Entity")]
        public bool IsEntityActive
        {
            get { return isEntityActive; }
            set { isEntityActive = value; }
        }

        [System.ComponentModel.Browsable(false)]
        public float TmpXCoordinate
        {
            get { return tmpXCoordinate; }
            set { tmpXCoordinate = value; }
        }
        

        [System.ComponentModel.Browsable(false)]
        public float TmpYCoordinate
        {
            get { return tmpYCoordinate; }
            set { tmpYCoordinate = value; }
        }
        private EntityContainer owningContainer;

        [System.ComponentModel.Browsable(false)]
        [System.ComponentModel.Description("The current decay factor assigned with this entity. 1 if no decay function active. Do not change."),
        System.ComponentModel.Category("Props of Selected Entity")]
        public float CurrentDecayFactor
        {
            get 
            {
                return CalculateCurrentDecayFactor(DateTime.Now);
            }
            set
            { 
                currentDecayFactor = value;
            }
        }

        public float GetVectorNorm()
        {
            return (float)Math.Sqrt(axisVal[0] * axisVal[0] + axisVal[1] * axisVal[1] + axisVal[2] * axisVal[2]);
        }

        public static float GetVectorNorm(float x, float y)
        {
            return GetVectorNorm(x, y, 0.0f);
        }

        public static float GetVectorNorm(float x, float y, float z)
        {
            return (float)Math.Sqrt(x * x + y * y + z * z);
        }

        public float CalculateCurrentDecayFactor(DateTime pointInTime)
        {
            TimeSpan dt = pointInTime - timeLastUpdatedGUI;
            float vecNorm = GetVectorNorm();
            float fac = (float)Math.Abs(vecNorm - vectorBase);
            float tmpLambda = 0.0f;
            float tempVal = 0.0f;
            float tmpDur = 0.0f;
            if (fac == 0.0f)
            {
                tmpLambda = 0.0f;
            }
            else
            {
                if (lambda > 0.0f)
                {
                    tmpLambda = (10.0f * lambda) / (fac * timeTillVectorBase);
                }
                else
                {
                    tmpLambda = 5.0f / (fac * timeTillVectorBase);
                }
            }
            switch (decayFunction)
            {
                case DecayFunctionType.NoDecay:
                    return vecNorm;
                    //break;
                case DecayFunctionType.Linear:
                    if (vecNorm > vectorBase)
                    {
                        tempVal = vecNorm - (float)dt.TotalSeconds * (1.0f / timeTillVectorBase);
                        if (tempVal < vectorBase)
                        {
                            return vectorBase;
                        }
                        else
                        {
                            return tempVal;
                        }
                    }
                    else
                    {
                        tempVal = vecNorm + (float)dt.TotalSeconds * (1.0f / timeTillVectorBase);
                        if (tempVal > vectorBase)
                        {
                            return vectorBase;
                        }
                        else
                        {
                            return tempVal;
                        }
                    }
                    //break;
                case DecayFunctionType.Exponential:
                    return vectorBase + (vecNorm - vectorBase) * (float)(Math.Exp(-tmpLambda * dt.TotalSeconds));
                    //break;
                case DecayFunctionType.Hyperbolic:
                    tmpDur = 0.5f * fac * timeTillVectorBase;
                    return vectorBase + ((vecNorm - vectorBase) / 2.0f) * (1.0f - (float)Math.Tanh(tmpLambda * (dt.TotalSeconds - tmpDur)));
                    //break;
                default:
                    break;
            }
            return 1.0f;
        }     

        public float this[string name]
        {
            get
            {
                for (int i = 0; i < owningContainer.AxisMapName.Length; ++i)
                {
                    if (owningContainer.AxisMapName[i] == name)
                    {
                        return axisVal[i];
                    }
                }
                return float.NaN;
            }
            set
            {
                for (int i = 0; i < owningContainer.AxisMapName.Length; ++i)
                {
                    if (owningContainer.AxisMapName[i] == name)
                    {
                        axisVal[i] = value;
                        return;
                    }
                }
                switch (name)
                {
                    case "ctl_decay":
                        decayFunction = (DecayFunctionType)(value+0.5f);
                        break;
                    case "ctl_id":
                        break;
                    case "ctl_timeLastUpdated":
                        //timeLastUpdated = value;
                        break;
                    case "ctl_weightFactor":
                        weightFactor = value;
                        break;
                    case "ctl_timeTillVectorBase":
                        timeTillVectorBase = value;
                        break;
                    case "ctl_lambda":
                        lambda = value;
                        break;
                    default:
                        break;
                }
            }
        }

        public float this[string name, DateTime updateTime]
        {
            get
            {
                for (int i = 0; i < owningContainer.AxisMapName.Length; ++i)
                {
                    if (owningContainer.AxisMapName[i] == name)
                    {
                        return axisVal[i];
                    }
                }
                return float.NaN;
            }
            set
            {
                for (int i = 0; i < owningContainer.AxisMapName.Length; ++i)
                {
                    if (owningContainer.AxisMapName[i] == name)
                    {
                        axisVal[i] = value;
                        timeLastUpdatedGUI = updateTime;
                        return;
                    }
                }
                switch (name)
                {
                    case "ctl_decay":
                        decayFunction = (DecayFunctionType)(value + 0.5f);
                        break;
                    case "ctl_id":
                        break;
                    case "ctl_timeLastUpdated":
                        //timeLastUpdated = value;
                        break;
                    case "ctl_weightFactor":
                        weightFactor = value;
                        break;
                    case "ctl_timeTillVectorBase":
                        timeTillVectorBase = value;
                        break;
                    case "ctl_lambda":
                        lambda = value;
                        break;
                    default:
                        break;
                }
            }
        }

        public float this[int index]
        {
            get
            {
                if (owningContainer.AxisMapName[index] == null || index < 0 || index > 2)
                    return float.NaN;
                return axisVal[index];
            }
            set
            {
                if (owningContainer.AxisMapName[index] == null || index < 0 || index > 2)
                    return;
                axisVal[index] = value;
            }
        }

        [System.ComponentModel.Description("The vector length the entity shall contract/expand to in case of an activated decay function."),
        System.ComponentModel.Category("Props of Selected Entity")]
        public float VectorBase
        {
            get { return vectorBase; }
            set { vectorBase = value; }
        }

        [System.ComponentModel.Description("Steepness of decay function"),
        System.ComponentModel.Category("Props of Selected Entity")]
        public float Lambda
        {
            get { return lambda; }
            set { lambda = value; }
        }

        [System.ComponentModel.Description("Timespan to move 1 unit to vector base in case of a decay function"),
        System.ComponentModel.Category("Props of Selected Entity")]
        public float TimeTillVectorBase
        {
            get
            { 
                return timeTillVectorBase; 
            }
            set
            {
                timeTillVectorBaseGUI = TimeSpan.FromSeconds(value);
                timeTillVectorBase = value;
            }
        }

        [System.ComponentModel.Browsable(false)]
        public TimeSpan TimeTillVectorBaseGUI
        {
            get 
            {
                return timeTillVectorBaseGUI;
            }
            set
            {
                timeTillVectorBase = (float)value.TotalSeconds;
                timeTillVectorBaseGUI = value; 
            }
        }

        [System.ComponentModel.Description("Corresponding weight factor of the entity that influences the calculation of the combined vector"),
        System.ComponentModel.Category("Props of Selected Entity")]
        public float WeightFactor
        {
            get { return weightFactor; }
            set { weightFactor = value; }
        }

        [System.ComponentModel.Description("The type of decay function for the corresponding entity"),
        System.ComponentModel.Category("Props of Selected Entity")]
        public DecayFunctionType DecayFunction
        {
            get { return decayFunction; }
            set { decayFunction = value; }
        }

        [System.ComponentModel.Browsable(false)]
        public string IdName
        {
            get { return idName; }
            set { idName = value; }
        }

        [System.ComponentModel.Browsable(false)]
        public DateTime TimeLastUpdatedGUI
        {
            get { return timeLastUpdatedGUI; }
            set { timeLastUpdatedGUI = value; }
        }

    }
}
