/******************************************************************************
'*
'*  (C) Copyright 2003 - 2026 Cougar Mountain Software
'*  All Rights reserved.
'*
'*  This program is an unpublished copyrighted work which is proprietary
'*  to Cougar Mountain Software and contains confidential information that
'*  is not to be reproduced or disclosed to any other person or entity
'*  without prior written consent from Cougar Mountain Software in each
'*  and every instance.
'*
'*  WARNING:  Unauthorized reproduction of this program as well as
'*  unauthorized preparation of derivative works based upon the
'*  program or distribution of copies by sale, rental, lease or
'*  lending are violations of federal copyright laws and state trade
'*  secret laws, punishable by civil and criminal penalties.
'*
'*****************************************************************************/

// Ignore Spelling: CMSUI frm REGVALUE MILISECOND

using System;
using System.Collections;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Threading;
using System.Windows.Forms;

namespace CMSUI
{
    /// <summary>
    /// Summary description for frmSplash.
    /// </summary>
    public class frmSplash : Form
    {
        static Thread ms_oThread = null;

        // Fade in and out.
        private double m_dblOpacityIncrement = .05;
        readonly double m_dblOpacityDecrement = .08;
        private const int TIMER_INTERVAL = 50;

        // Status and progress bar
        static string ms_sStatus;
        private double m_dblCompletionFraction = 0;
        private Rectangle m_rProgress;
        private static bool started = false;

        // Progress smoothing
        private double m_dblLastCompletionFraction = 0.0;
        private double m_dblPBIncrementPerTimerInterval = .015;

        private bool m_bFirstLaunch = false;
        private DateTime m_dtStart;
        private bool m_bDTSet = false;
        private int m_iIndex = 1;
        private int m_iActualTicks = 0;
        private ArrayList m_alPreviousCompletionFraction;
        private ArrayList m_alActualTimes = new ArrayList();
        private const string REG_KEY_INITIALIZATION = "Initialization";
        private const string REGVALUE_PB_MILISECOND_INCREMENT = "Increment";
        private const string REGVALUE_PB_PERCENTS = "Percents";

        private Panel pnlContainer;
        private PictureBox pbSideBMP;
        private Label lblProduct;
        private Label label1;
        private Label lblCopyright;
        private Panel pnlStatus;
        private Label lblStatus;
        private Label lblTimeRemaining;
        private System.Windows.Forms.Timer timer1;
        private IContainer components;

        /// <summary>
        /// Initializes a new instance of the <see cref="frmSplash"/> class.
        /// </summary>
        public frmSplash()
        {
            InitializeComponent();

            Opacity = .00;
            timer1.Interval = TIMER_INTERVAL;
            timer1.Start();
        }

        /// <summary>
        /// Shows the splash screen.
        /// </summary>
        public static void ShowSplashScreen()
        {
            if (SplashScreen != null)
                return;

            ms_oThread = new Thread(new ThreadStart(ShowForm))
            {
                IsBackground = true
            };
            ms_oThread.SetApartmentState(ApartmentState.STA);
            ms_oThread.Start();

            while (started == false)
            {
            }
        }

        /// <summary>
        /// Gets the splash screen.
        /// </summary>
        /// <value>
        /// The splash screen.
        /// </value>
        static public frmSplash SplashScreen { get; private set; } = null;

        /// <summary>
        /// Shows the form.
        /// </summary>
        static private void ShowForm()
        {
            SplashScreen = new frmSplash();
            Application.Run(SplashScreen);
        }

        /// <summary>
        /// Closes the form.
        /// </summary>
        static public void CloseForm()
        {
            if (SplashScreen != null && SplashScreen.IsDisposed == false)
            {
                // just make the splash screen start going away
                SplashScreen.m_dblOpacityIncrement = -SplashScreen.m_dblOpacityDecrement;
                Thread.Sleep(500);
            }
            ms_oThread = null;
            SplashScreen = null;
        }

        /// <summary>
        /// Sets the status.
        /// </summary>
        /// <param name="newStatus">The new status.</param>
        static public void SetStatus(string newStatus)
        {
            SetStatus(newStatus, true);
        }

        /// <summary>
        /// A static method to set the status and optionally update the reference.
        /// This is useful if you are in a section of code that has a variable
        /// set of status string updates. In that case, don't set the reference
        /// </summary>
        /// <param name="newStatus"></param>
        /// <param name="setReference"></param>
        static public void SetStatus(string newStatus, bool setReference)
        {
            ms_sStatus = newStatus;
            if (SplashScreen == null)
                return;
            if (setReference)
                SplashScreen.SetReferenceInternal();
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                components?.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code =============================

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new Container();
            ComponentResourceManager resources = new ComponentResourceManager(typeof(frmSplash));
            this.pnlContainer = new Panel();
            this.lblTimeRemaining = new Label();
            this.lblStatus = new Label();
            this.pnlStatus = new Panel();
            this.lblCopyright = new Label();
            this.label1 = new Label();
            this.lblProduct = new Label();
            this.pbSideBMP = new PictureBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.pnlContainer.SuspendLayout();
            ((ISupportInitialize)this.pbSideBMP).BeginInit();
            this.SuspendLayout();
            //
            // pnlContainer
            //
            this.pnlContainer.BackColor = Color.WhiteSmoke;
            this.pnlContainer.Controls.Add(this.lblTimeRemaining);
            this.pnlContainer.Controls.Add(this.lblStatus);
            this.pnlContainer.Controls.Add(this.pnlStatus);
            this.pnlContainer.Controls.Add(this.lblCopyright);
            this.pnlContainer.Controls.Add(this.label1);
            this.pnlContainer.Controls.Add(this.lblProduct);
            this.pnlContainer.Controls.Add(this.pbSideBMP);
            this.pnlContainer.Dock = System.Windows.Forms.DockStyle.Top;
            this.pnlContainer.Location = new Point(0, 0);
            this.pnlContainer.Name = "pnlContainer";
            this.pnlContainer.Size = new Size(416, 416);
            this.pnlContainer.TabIndex = 0;
            this.pnlContainer.Paint += new PaintEventHandler(this.Container_Paint);
            //
            // lblTimeRemaining
            //
            this.lblTimeRemaining.Location = new Point(144, 240);
            this.lblTimeRemaining.Name = "lblTimeRemaining";
            this.lblTimeRemaining.Size = new Size(264, 23);
            this.lblTimeRemaining.TabIndex = 6;
            this.lblTimeRemaining.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblTimeRemaining.Click += new EventHandler(this.TimeRemaining_Click);
            //
            // lblStatus
            //
            this.lblStatus.Location = new Point(144, 184);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new Size(256, 16);
            this.lblStatus.TabIndex = 5;
            //
            // pnlStatus
            //
            this.pnlStatus.Location = new Point(144, 208);
            this.pnlStatus.Name = "pnlStatus";
            this.pnlStatus.Size = new Size(264, 24);
            this.pnlStatus.TabIndex = 4;
            this.pnlStatus.Paint += new PaintEventHandler(this.Status_Paint);
            //
            // lblCopyright
            //
            this.lblCopyright.Location = new Point(144, 368);
            this.lblCopyright.Name = "lblCopyright";
            this.lblCopyright.Size = new Size(264, 32);
            this.lblCopyright.TabIndex = 3;
            this.lblCopyright.Text = "Copyright 2005-2026 by Cougar Mountain Software";
            //
            // label1
            //
            this.label1.Location = new Point(144, 40);
            this.label1.Name = "label1";
            this.label1.Size = new Size(264, 23);
            this.label1.TabIndex = 2;
            this.label1.Text = "Operations Support Management Tool";
            //
            // lblProduct
            //
            this.lblProduct.Font = new Font("Tahoma", 10.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, 0);
            this.lblProduct.Location = new Point(144, 16);
            this.lblProduct.Name = "lblProduct";
            this.lblProduct.Size = new Size(264, 24);
            this.lblProduct.TabIndex = 1;
            this.lblProduct.Text = "Denali by Cougar Mountain";
            //
            // pbSideBMP
            //
            this.pbSideBMP.Image = (Image)resources.GetObject("pbSideBMP.Image");
            this.pbSideBMP.Location = new Point(8, 8);
            this.pbSideBMP.Name = "pbSideBMP";
            this.pbSideBMP.Size = new Size(124, 402);
            this.pbSideBMP.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pbSideBMP.TabIndex = 0;
            this.pbSideBMP.TabStop = false;
            //
            // timer1
            //
            this.timer1.Tick += new EventHandler(this.Timer1_Tick);
            //
            // frmSplash
            //
            this.AutoScaleBaseSize = new Size(5, 14);
            this.ClientSize = new Size(416, 414);
            this.Controls.Add(this.pnlContainer);
            this.Font = new Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "frmSplash";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "frmSplash";
            this.DoubleClick += new EventHandler(this.Splash_DoubleClick);
            this.pnlContainer.ResumeLayout(false);
            this.pnlContainer.PerformLayout();
            ((ISupportInitialize)this.pbSideBMP).EndInit();
            this.ResumeLayout(false);

        }
        #endregion

        // ******************* Static Methods *********************** //

        // Static method called from the initializing application to
        // give the splash screen reference points.  Not needed if
        // you are using a lot of status strings.
        /// <summary>
        /// Sets the reference point.
        /// </summary>
        static public void SetReferencePoint()
        {
            if (SplashScreen == null)
                return;
            SplashScreen.SetReferenceInternal();

        }

        // ************ Private methods ************

        // Internal method for setting reference points.
        /// <summary>
        /// Sets the reference internal.
        /// </summary>
        private void SetReferenceInternal()
        {
            if (m_bDTSet == false)
            {
                m_bDTSet = true;
                m_dtStart = DateTime.Now;
                ReadIncrements();
            }
            double dblMilliseconds = ElapsedMilliseconds();
            m_alActualTimes.Add(dblMilliseconds);
            m_dblLastCompletionFraction = m_dblCompletionFraction;
            m_dblCompletionFraction = m_alPreviousCompletionFraction != null && m_iIndex < m_alPreviousCompletionFraction.Count
                ? (double)m_alPreviousCompletionFraction[m_iIndex++]
                : (m_iIndex > 0) ? 1 : 0;
        }

        // Utility function to return elapsed Milliseconds since the
        // SplashScreen was launched.
        /// <summary>
        /// Elapsed the milliseconds.
        /// </summary>
        /// <returns></returns>
        private double ElapsedMilliseconds()
        {
            TimeSpan ts = DateTime.Now - m_dtStart;
            return ts.TotalMilliseconds;
        }

        // Function to read the checkpoint intervals from the previous invocation of the
        // splash screen from the registry.
        /// <summary>
        /// Reads the increments.
        /// </summary>
        private void ReadIncrements()
        {
            string sPBIncrementPerTimerInterval = RegistryAccess.GetStringRegistryValue(REGVALUE_PB_MILISECOND_INCREMENT, "0.0015");

            m_dblPBIncrementPerTimerInterval = Double.TryParse(sPBIncrementPerTimerInterval, System.Globalization.NumberStyles.Float, System.Globalization.NumberFormatInfo.InvariantInfo, out double dblResult) == true
                ? dblResult
                : .0015;

            string sPBPreviousPctComplete = RegistryAccess.GetStringRegistryValue(REGVALUE_PB_PERCENTS, "");

            if (sPBPreviousPctComplete != "")
            {
                string[] aTimes = sPBPreviousPctComplete.Split(null);
                m_alPreviousCompletionFraction = new ArrayList();

                for (int i = 0; i < aTimes.Length; i++)
                {
                    if (Double.TryParse(aTimes[i], System.Globalization.NumberStyles.Float, System.Globalization.NumberFormatInfo.InvariantInfo, out double dblVal))
                        m_alPreviousCompletionFraction.Add(dblVal);
                    else
                        m_alPreviousCompletionFraction.Add(1.0);
                }
            }
            else
            {
                m_bFirstLaunch = true;
                lblTimeRemaining.Text = "";
            }
        }

        // Method to store the intervals (in percent complete) from the current invocation of
        // the splash screen to the registry.
        /// <summary>
        /// Stores the increments.
        /// </summary>
        private void StoreIncrements()
        {
            string sPercent = "";
            double dblElapsedMilliseconds = ElapsedMilliseconds();
            for (int i = 0; i < m_alActualTimes.Count; i++)
                sPercent += ((double)m_alActualTimes[i] / dblElapsedMilliseconds).ToString("0.####", System.Globalization.NumberFormatInfo.InvariantInfo) + " ";

            RegistryAccess.SetStringRegistryValue(REGVALUE_PB_PERCENTS, sPercent);

            m_dblPBIncrementPerTimerInterval = 1.0 / m_iActualTicks;
            RegistryAccess.SetStringRegistryValue(REGVALUE_PB_MILISECOND_INCREMENT, m_dblPBIncrementPerTimerInterval.ToString("#.000000", System.Globalization.NumberFormatInfo.InvariantInfo));
        }

        //********* Event Handlers ************

        /// <summary>
        /// Handles the Tick event of the timer1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void Timer1_Tick(
            object sender,
            EventArgs e)
        {

            lblStatus.Text = ms_sStatus;

            if (m_dblOpacityIncrement > 0)
            {
                m_iActualTicks++;
                if (Opacity < 1)
                    Opacity += m_dblOpacityIncrement;
            }
            else
            {
                if (Opacity > 0)
                    Opacity += m_dblOpacityIncrement;
                else
                {
                    StoreIncrements();
                    Close();
                    Debug.WriteLine("Called this.Close()");
                }
            }
            if (m_bFirstLaunch == false && m_dblLastCompletionFraction < m_dblCompletionFraction)
            {
                m_dblLastCompletionFraction += m_dblPBIncrementPerTimerInterval;
                int width = (int)Math.Floor(pnlStatus.ClientRectangle.Width * m_dblLastCompletionFraction);
                int height = pnlStatus.ClientRectangle.Height;
                int x = pnlStatus.ClientRectangle.X;
                int y = pnlStatus.ClientRectangle.Y;
                if (width > 0 && height > 0)
                {
                    m_rProgress = new Rectangle(x, y, width, height);
                    pnlStatus.Invalidate(m_rProgress);
                    int iSecondsLeft = 1 + (int)(TIMER_INTERVAL * ((1.0 - m_dblLastCompletionFraction) / m_dblPBIncrementPerTimerInterval)) / 1000;
                    lblTimeRemaining.Text = iSecondsLeft == 1 ? string.Format("1 second remaining") : string.Format("{0} seconds remaining", iSecondsLeft);

                }
            }

        }

        /// <summary>
        /// Handles the Paint event of the pnlStatus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="PaintEventArgs"/> instance containing the event data.</param>
        private void Status_Paint(
            object sender,
            PaintEventArgs e)
        {
            if (m_bFirstLaunch == false && e.ClipRectangle.Width > 0 && m_iActualTicks > 1)
            {
                //LinearGradientBrush brBackground = new LinearGradientBrush(m_rProgress, Color.FromArgb(58, 96, 151),
                //	Color.FromArgb(181, 237, 254), LinearGradientMode.Horizontal);

                LinearGradientBrush brBackground = new LinearGradientBrush(m_rProgress, Color.Goldenrod,
                    Color.Gold, LinearGradientMode.Horizontal);
                e.Graphics.FillRectangle(brBackground, m_rProgress);
            }
        }

        /// <summary>
        /// Handles the DoubleClick event of the frmSplash control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void Splash_DoubleClick(
            object sender,
            EventArgs e)
        {
            CloseForm();
        }

        /// <summary>
        /// </summary>
        /// <param name="e">A <see cref="T:System.Windows.Forms.PaintEventArgs" /> that contains the event data.</param>
        protected override void OnPaint(
            PaintEventArgs e)
        {
            base.OnPaint(e);
            started = true;
        }

        /// <summary>
        /// Handles the Click event of the lblTimeRemaining control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void TimeRemaining_Click(
            object sender,
            EventArgs e)
        {

        }

        /// <summary>
        /// Handles the Paint event of the pnlContainer control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="PaintEventArgs"/> instance containing the event data.</param>
        private void Container_Paint(
            object sender,
            PaintEventArgs e)
        {

        }
    }
}
