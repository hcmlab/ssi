using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FusionUI
{
    public partial class EntityCreatorForm : Form
    {
        public EntityCreatorForm(EntityContainer _owningContainer, string _defaultEntityName)
        {
            InitializeComponent();
            owningContainer = _owningContainer;
            Entity_id = defaultEntityName = _defaultEntityName;
        }

        private Entity entityToCreate;
        private string entity_id;
        private string defaultEntityName;
        public string Entity_id
        {
            get { return entity_id; }
            set { entity_id = value; }
        }

        public Entity EntityToCreate
        {
            get { return entityToCreate; }
            set { entityToCreate = value; }
        }
        private EntityContainer owningContainer;

        private void EntityCreator_Load(object sender, EventArgs e)
        {
            entityToCreate = new Entity(owningContainer);
            propertyGrid1.SelectedObject = entityToCreate;
            textBox1.Text = Entity_id;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            entityToCreate = new Entity(owningContainer);
            propertyGrid1.SelectedObject = entityToCreate;
            textBox1.Text = Entity_id = defaultEntityName;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            Entity_id = textBox1.Text;
        }
    }
}
