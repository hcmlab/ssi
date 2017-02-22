using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FusionUI
{
    public class EntityContainer
    {
        public List<Entity> entitySafe;

        private string[] axisMapName = new string[3];
        private float contributionThreshold = 0.1f;
        private float speed = 0.1f;
        public float[] combinedVec = new float[3];
        public float[] modVec = new float[3];
        public float[] massCenter = new float[3];

        public float overAllWeight = 0.0f;
        public float[] tmpDecayedVector = new float[3];
        public float decayFactor = 0.0f;
        public float tmpNorm = 0.0f;
        public float decayedNorm = 0.0f;
        public int numberOfContributingVectors = 0;

        public EntityContainer()
        {
            //entitySafe = new Dictionary<string, Entity>();
            entitySafe = new List<Entity>();
            axisMapName[0] = "p";
            axisMapName[1] = "a";
        }

        public float Speed
        {
            get { return speed; }
            set { speed = value; }
        }

        public float ContributionThreshold
        {
            get { return contributionThreshold; }
            set { contributionThreshold = value; }
        }

        public string[] AxisMapName
        {
            get
            {
                return axisMapName;
            }
            set
            {
                axisMapName = value;
            }
        }

        public bool NameSpecificAxis(int axisIndex, string newName)
        {
            if (axisIndex < 0 || axisIndex > 2)
            {
                return false;
            }
            if (newName == string.Empty)
                newName = null;
            if (axisMapName[axisIndex] == null)
            {
                axisMapName[axisIndex] = newName;
                return true;
            }
            else
            {
                axisMapName[axisIndex] = newName;
                return true;
            }
        }

        public int GetIndexOfSpecificAxis(string name)
        {
            int i = 0;
            foreach (string tmpStr in axisMapName)
            {
                if (tmpStr == name)
                {
                    return i;
                }
                ++i;
            }

            return -1;
        }

        

    }
}

