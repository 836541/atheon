
namespace Guardian
{
    partial class Guardian
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Guardian));
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.pctDiminuir = new System.Windows.Forms.PictureBox();
            this.pctX = new System.Windows.Forms.PictureBox();
            this.pctMenor = new System.Windows.Forms.PictureBox();
            this.pctMaior = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctDiminuir)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctMenor)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctMaior)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Cursor = System.Windows.Forms.Cursors.Hand;
            this.pictureBox1.Image = global::Guardian.Properties.Resources.Botão_principal;
            this.pictureBox1.Location = new System.Drawing.Point(225, 184);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(202, 202);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBox1.TabIndex = 3;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.Click += new System.EventHandler(this.pictureBox1_Click_1);
            // 
            // pctDiminuir
            // 
            this.pctDiminuir.Cursor = System.Windows.Forms.Cursors.Hand;
            this.pctDiminuir.Image = global::Guardian.Properties.Resources.Diminuir;
            this.pctDiminuir.Location = new System.Drawing.Point(576, 35);
            this.pctDiminuir.Name = "pctDiminuir";
            this.pctDiminuir.Size = new System.Drawing.Size(16, 4);
            this.pctDiminuir.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pctDiminuir.TabIndex = 2;
            this.pctDiminuir.TabStop = false;
            this.pctDiminuir.Click += new System.EventHandler(this.pctDiminuir_Click);
            // 
            // pctX
            // 
            this.pctX.Cursor = System.Windows.Forms.Cursors.Hand;
            this.pctX.Image = global::Guardian.Properties.Resources.Xzinho;
            this.pctX.Location = new System.Drawing.Point(614, 21);
            this.pctX.Name = "pctX";
            this.pctX.Size = new System.Drawing.Size(15, 18);
            this.pctX.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pctX.TabIndex = 0;
            this.pctX.TabStop = false;
            this.pctX.Click += new System.EventHandler(this.pctX_Click);
            // 
            // pctMenor
            // 
            this.pctMenor.Image = global::Guardian.Properties.Resources.parte_maneira_pequena_2;
            this.pctMenor.Location = new System.Drawing.Point(454, 321);
            this.pctMenor.Name = "pctMenor";
            this.pctMenor.Size = new System.Drawing.Size(245, 285);
            this.pctMenor.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pctMenor.TabIndex = 1;
            this.pctMenor.TabStop = false;
            // 
            // pctMaior
            // 
            this.pctMaior.Image = global::Guardian.Properties.Resources.parte_maneira_grande_2;
            this.pctMaior.Location = new System.Drawing.Point(-5, -47);
            this.pctMaior.Name = "pctMaior";
            this.pctMaior.Size = new System.Drawing.Size(556, 585);
            this.pctMaior.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pctMaior.TabIndex = 0;
            this.pctMaior.TabStop = false;
            // 
            // Guardian
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(650, 550);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.pctDiminuir);
            this.Controls.Add(this.pctX);
            this.Controls.Add(this.pctMenor);
            this.Controls.Add(this.pctMaior);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Guardian";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Atheon™";
            this.Load += new System.EventHandler(this.Guardian_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctDiminuir)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctMenor)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pctMaior)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pctMaior;
        private System.Windows.Forms.PictureBox pctMenor;
        private System.Windows.Forms.PictureBox pctX;
        private System.Windows.Forms.PictureBox pctDiminuir;
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}

