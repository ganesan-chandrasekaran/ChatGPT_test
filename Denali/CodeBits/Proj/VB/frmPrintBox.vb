'****************************************************
'*
'*  (C) Copyright 2003-2026 Cougar Mountain Software
'*  All Rights reserved.
'*
'*  This program is an unpublished copyrighted work
'*  which is proprietary to Cougar Mountain Software
'*  and contains confidential information that is not
'*  to be reproduced or disclosed to any other person
'*  or entity without prior written consent from
'*  Cougar Mountain Software in each and every
'*  instance.
'*
'*  WARNING:  Unauthorized reproduction of this
'*  program as well as unauthorized preparation of
'*  derivative works based upon the program or
'*  distribution of copies by sale, rental, lease
'*  or lending are violations of federal copyright
'*  laws and state trade secret laws, punishable by
'*  civil and criminal penalties.
'*
'****************************************************

#Region " Imports ================================================================ "

Imports NETDLL.clsServerCommandSender
Imports CrystalDecisions
Imports CrystalDecisions.CrystalReports.Engine
Imports CrystalDecisions.Shared
Imports System.Runtime.InteropServices.ComVisibleAttribute
Imports System.Runtime.InteropServices.RegistrationServices
Imports System.Reflection

#End Region

Friend Class frmPrintBox

  Inherits Form

  Private WithEvents m_objReport As New ReportDocument
  Private bCancel As Boolean = False   'cancel variable for wait dialog and wait thread AES
  Private m_ReportLocation As String
  Private m_bDeveloperViewer As Boolean
  Private m_strXMLParameters As String
  Private m_strCommand As String
  Private m_strData As String
  Private m_nGrpOption As Integer
  Private m_nGrpPrintType As Integer
  Private m_nReportType As enuReportType
  Private m_strCopies As String
  Private m_strXMLPrintParms As String
  Private m_intOUTPUTTYPE As Integer
  Private m_strNUMBEROFCOPIES As String
  Private m_intFORMAT As Integer
  Private m_intEXPORT As Integer   'Used with emu for the state the form is in
  Private m_strPRINTER As String
  Private m_bEdit As Boolean
  Private m_strErrorXML As String
  Private m_bInProcess As Boolean = False
  Private m_strFileName, m_strFilePath, m_szDiskFileName, m_strReportName As String
  Private m_bPRINT_REPORT_PARAMETERS As Boolean = False
  Private m_hwPrintScreen As Long = 0

  '*** Properties
  '// DG, 3/28/2003, Defect 1-4057, Created new propery to set/get report title.
  Friend m_strReportTitle As String

  Friend WriteOnly Property DeveloperViewer() As Boolean

    Set(ByVal Value As Boolean)

      m_bDeveloperViewer = Value

    End Set

  End Property

  Friend WriteOnly Property XMLParameters() As String

    Set(ByVal Value As String)

      m_strXMLParameters = Value

    End Set

  End Property

  Friend WriteOnly Property Command() As String

    Set(ByVal Value As String)

      m_strCommand = Value

    End Set

  End Property

  Friend WriteOnly Property Data() As String

    Set(ByVal Value As String)

      m_strData = Value

    End Set

  End Property

  Friend ReadOnly Property ShowPrintDialog() As Boolean

    Get

      Return m_bEdit

    End Get

  End Property

  Friend ReadOnly Property ScreenHandle() As Long

    Get

      Return m_hwPrintScreen

    End Get

  End Property

  Friend Property ReportTitle() As String

    Get

      Return m_strReportTitle

    End Get

    Set(ByVal Value As String)

      m_strReportTitle = Value

    End Set

  End Property  ' Report Title

  Private m_XMLDataset As New DataSet

  'enums for Report types from printer prarms
  Friend Enum enuOutputType
    OT_Printer = 1
    OT_Screen = 2
    OT_File = 3
  End Enum

  'enums for Format types from printer prarms
  Friend Enum enuFormatType
    FT_Dot = 2
    FT_Laser = 1
  End Enum

  'enums for dialog state for exporting
  Friend Enum enuExportState
    EX_NonOption = 0
    EX_Option = 1
  End Enum

  'enums for dialog state for exporting
  Friend Enum enuExportFormat
    WORD = 0
    EXCEL = 1
    PDF = 2
    HTML40 = 3
    XML = 4
  End Enum

  'enums for the report types add 6.16.04 AES
  Friend Enum enuReportType
    RT_Report = 1
    RT_Posting = 2
    RT_PayrollChecks = 3
    RT_ImmediateChecks = 4
    RT_PayBillsChecks = 5
    RT_PurchaseOrders = 6
    RT_Statements = 7
    RT_Checks = 8
    RT_CombinedReports = 9
  End Enum

#Region " Windows Form Designer generated code "

  Friend Sub New()

    MyBase.New()

    'This call is required by the Windows Form Designer.
    InitializeComponent()

    'Add any initialization after the InitializeComponent() call

  End Sub

  'Form overrides dispose to clean up the component list.
  Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)

    If disposing Then
      If Not (components Is Nothing) Then
        components.Dispose()
      End If
    End If

    MyBase.Dispose(disposing)

  End Sub

  'Required by the Windows Form Designer
  Private components As System.ComponentModel.IContainer

  'NOTE: The following procedure is required by the Windows Form Designer
  'It can be modified using the Windows Form Designer.
  'Do not modify it using the code editor.
  Friend WithEvents lblTitle As Label
  Friend WithEvents grpPrintOptions As GroupBox
  Friend WithEvents rdoFile As RadioButton
  Friend WithEvents rdoScreen As RadioButton
  Friend WithEvents rdoPrinter As RadioButton
  Friend WithEvents rdoDot As RadioButton
  Friend WithEvents rdoLaser As RadioButton
  Friend WithEvents txtCopies As TextBox
  Friend WithEvents lblCopies As Label
  Friend WithEvents txtFileLocation As TextBox
  Friend WithEvents lblPath As Label
  Friend WithEvents lblPrintLocation As Label
  Friend WithEvents GroupBox2 As GroupBox
  Friend WithEvents btnSetPrinter As Button
  Friend WithEvents btnCancel As Button
  Friend WithEvents btnOk As Button
  Friend WithEvents lblExportType As Label
  Friend WithEvents cboExportType As ComboBox
  Friend WithEvents grpPrintFormat As GroupBox
  Friend WithEvents grpTopLine As GroupBox
  Friend WithEvents grpBottomLine As GroupBox
  Friend WithEvents PictureBox1 As PictureBox
  Friend WithEvents chkPrintReportParameters As System.Windows.Forms.CheckBox

  <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
    Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(frmPrintBox))
    Me.lblTitle = New System.Windows.Forms.Label
    Me.grpTopLine = New System.Windows.Forms.GroupBox
    Me.grpPrintOptions = New System.Windows.Forms.GroupBox
    Me.lblExportType = New System.Windows.Forms.Label
    Me.cboExportType = New System.Windows.Forms.ComboBox
    Me.rdoFile = New System.Windows.Forms.RadioButton
    Me.rdoScreen = New System.Windows.Forms.RadioButton
    Me.rdoPrinter = New System.Windows.Forms.RadioButton
    Me.grpPrintFormat = New System.Windows.Forms.GroupBox
    Me.rdoDot = New System.Windows.Forms.RadioButton
    Me.rdoLaser = New System.Windows.Forms.RadioButton
    Me.txtCopies = New System.Windows.Forms.TextBox
    Me.lblCopies = New System.Windows.Forms.Label
    Me.txtFileLocation = New System.Windows.Forms.TextBox
    Me.lblPath = New System.Windows.Forms.Label
    Me.lblPrintLocation = New System.Windows.Forms.Label
    Me.grpBottomLine = New System.Windows.Forms.GroupBox
    Me.GroupBox2 = New System.Windows.Forms.GroupBox
    Me.btnSetPrinter = New System.Windows.Forms.Button
    Me.btnCancel = New System.Windows.Forms.Button
    Me.btnOk = New System.Windows.Forms.Button
    Me.PictureBox1 = New System.Windows.Forms.PictureBox
    Me.chkPrintReportParameters = New System.Windows.Forms.CheckBox
    Me.grpPrintOptions.SuspendLayout()
    Me.grpPrintFormat.SuspendLayout()
    Me.grpBottomLine.SuspendLayout()
    Me.SuspendLayout()
    '
    'lblTitle
    '
    Me.lblTitle.Font = New System.Drawing.Font("Arial", 8.25!)
    Me.lblTitle.Location = New System.Drawing.Point(55, 8)
    Me.lblTitle.Name = "lblTitle"
    Me.lblTitle.Size = New System.Drawing.Size(287, 36)
    Me.lblTitle.TabIndex = 12
    '
    'grpTopLine
    '
    Me.grpTopLine.Location = New System.Drawing.Point(6, 46)
    Me.grpTopLine.Name = "grpTopLine"
    Me.grpTopLine.Size = New System.Drawing.Size(336, 7)
    Me.grpTopLine.TabIndex = 13
    Me.grpTopLine.TabStop = False
    '
    'grpPrintOptions
    '
    Me.grpPrintOptions.Controls.Add(Me.lblExportType)
    Me.grpPrintOptions.Controls.Add(Me.cboExportType)
    Me.grpPrintOptions.Controls.Add(Me.rdoFile)
    Me.grpPrintOptions.Controls.Add(Me.rdoScreen)
    Me.grpPrintOptions.Controls.Add(Me.rdoPrinter)
    Me.grpPrintOptions.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.grpPrintOptions.Location = New System.Drawing.Point(5, 58)
    Me.grpPrintOptions.Name = "grpPrintOptions"
    Me.grpPrintOptions.Size = New System.Drawing.Size(336, 71)
    Me.grpPrintOptions.TabIndex = 0
    Me.grpPrintOptions.TabStop = False
    '
    'lblExportType
    '
    Me.lblExportType.Location = New System.Drawing.Point(8, 40)
    Me.lblExportType.Name = "lblExportType"
    Me.lblExportType.Size = New System.Drawing.Size(80, 21)
    Me.lblExportType.TabIndex = 24
    Me.lblExportType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
    '
    'cboExportType
    '
    Me.cboExportType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
    Me.cboExportType.Location = New System.Drawing.Point(88, 40)
    Me.cboExportType.Name = "cboExportType"
    Me.cboExportType.Size = New System.Drawing.Size(136, 21)
    Me.cboExportType.TabIndex = 4
    '
    'rdoFile
    '
    Me.rdoFile.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.rdoFile.Location = New System.Drawing.Point(166, 16)
    Me.rdoFile.Name = "rdoFile"
    Me.rdoFile.Size = New System.Drawing.Size(64, 16)
    Me.rdoFile.TabIndex = 2
    '
    'rdoScreen
    '
    Me.rdoScreen.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.rdoScreen.Location = New System.Drawing.Point(87, 16)
    Me.rdoScreen.Name = "rdoScreen"
    Me.rdoScreen.Size = New System.Drawing.Size(64, 16)
    Me.rdoScreen.TabIndex = 1
    '
    'rdoPrinter
    '
    Me.rdoPrinter.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.rdoPrinter.Location = New System.Drawing.Point(8, 16)
    Me.rdoPrinter.Name = "rdoPrinter"
    Me.rdoPrinter.Size = New System.Drawing.Size(64, 16)
    Me.rdoPrinter.TabIndex = 0
    '
    'grpPrintFormat
    '
    Me.grpPrintFormat.Controls.Add(Me.rdoDot)
    Me.grpPrintFormat.Controls.Add(Me.rdoLaser)
    Me.grpPrintFormat.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.grpPrintFormat.Location = New System.Drawing.Point(148, 164)
    Me.grpPrintFormat.Name = "grpPrintFormat"
    Me.grpPrintFormat.Size = New System.Drawing.Size(198, 35)
    Me.grpPrintFormat.TabIndex = 7
    Me.grpPrintFormat.TabStop = False
    Me.grpPrintFormat.Visible = False
    '
    'rdoDot
    '
    Me.rdoDot.Enabled = False
    Me.rdoDot.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.rdoDot.Location = New System.Drawing.Point(92, 12)
    Me.rdoDot.Name = "rdoDot"
    Me.rdoDot.Size = New System.Drawing.Size(96, 16)
    Me.rdoDot.TabIndex = 1
    Me.rdoDot.Visible = False
    '
    'rdoLaser
    '
    Me.rdoLaser.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.rdoLaser.Location = New System.Drawing.Point(8, 12)
    Me.rdoLaser.Name = "rdoLaser"
    Me.rdoLaser.Size = New System.Drawing.Size(76, 16)
    Me.rdoLaser.TabIndex = 0
    Me.rdoLaser.Visible = False
    '
    'txtCopies
    '
    Me.txtCopies.Location = New System.Drawing.Point(104, 173)
    Me.txtCopies.Name = "txtCopies"
    Me.txtCopies.Size = New System.Drawing.Size(32, 20)
    Me.txtCopies.TabIndex = 6
    Me.txtCopies.Text = String.Empty
    '
    'lblCopies
    '
    Me.lblCopies.Location = New System.Drawing.Point(7, 174)
    Me.lblCopies.Name = "lblCopies"
    Me.lblCopies.Size = New System.Drawing.Size(96, 20)
    Me.lblCopies.TabIndex = 18
    Me.lblCopies.TextAlign = System.Drawing.ContentAlignment.MiddleRight
    '
    'txtFileLocation
    '
    Me.txtFileLocation.Location = New System.Drawing.Point(6, 137)
    Me.txtFileLocation.Name = "txtFileLocation"
    Me.txtFileLocation.Size = New System.Drawing.Size(336, 20)
    Me.txtFileLocation.TabIndex = 5
    Me.txtFileLocation.Text = String.Empty
    '
    'lblPath
    '
    Me.lblPath.Location = New System.Drawing.Point(7, 228)
    Me.lblPath.Name = "lblPath"
    Me.lblPath.Size = New System.Drawing.Size(96, 16)
    Me.lblPath.TabIndex = 26
    '
    'lblPrintLocation
    '
    Me.lblPrintLocation.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
    Me.lblPrintLocation.Location = New System.Drawing.Point(104, 228)
    Me.lblPrintLocation.Name = "lblPrintLocation"
    Me.lblPrintLocation.Size = New System.Drawing.Size(240, 16)
    Me.lblPrintLocation.TabIndex = 25
    Me.lblPrintLocation.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
    '
    'grpBottomLine
    '
    Me.grpBottomLine.Controls.Add(Me.GroupBox2)
    Me.grpBottomLine.Location = New System.Drawing.Point(7, 244)
    Me.grpBottomLine.Name = "grpBottomLine"
    Me.grpBottomLine.Size = New System.Drawing.Size(336, 7)
    Me.grpBottomLine.TabIndex = 24
    Me.grpBottomLine.TabStop = False
    '
    'GroupBox2
    '
    Me.GroupBox2.Location = New System.Drawing.Point(0, 0)
    Me.GroupBox2.Name = "GroupBox2"
    Me.GroupBox2.Size = New System.Drawing.Size(336, 7)
    Me.GroupBox2.TabIndex = 11
    Me.GroupBox2.TabStop = False
    '
    'btnSetPrinter
    '
    Me.btnSetPrinter.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.btnSetPrinter.Font = New System.Drawing.Font("Arial", 8.25!)
    Me.btnSetPrinter.Location = New System.Drawing.Point(256, 260)
    Me.btnSetPrinter.Name = "btnSetPrinter"
    Me.btnSetPrinter.Size = New System.Drawing.Size(87, 26)
    Me.btnSetPrinter.TabIndex = 10
    '
    'btnCancel
    '
    Me.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel
    Me.btnCancel.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.btnCancel.Font = New System.Drawing.Font("Arial", 8.25!)
    Me.btnCancel.Location = New System.Drawing.Point(164, 260)
    Me.btnCancel.Name = "btnCancel"
    Me.btnCancel.Size = New System.Drawing.Size(87, 26)
    Me.btnCancel.TabIndex = 9
    '
    'btnOk
    '
    Me.btnOk.FlatStyle = System.Windows.Forms.FlatStyle.System
    Me.btnOk.Font = New System.Drawing.Font("Arial", 8.25!)
    Me.btnOk.Location = New System.Drawing.Point(72, 260)
    Me.btnOk.Name = "btnOk"
    Me.btnOk.Size = New System.Drawing.Size(87, 26)
    Me.btnOk.TabIndex = 8
    Me.btnOk.Text = "&"
    '
    'PictureBox1
    '
    Me.PictureBox1.Image = CType(resources.GetObject("PictureBox1.Image"), System.Drawing.Image)
    Me.PictureBox1.Location = New System.Drawing.Point(8, 8)
    Me.PictureBox1.Name = "PictureBox1"
    Me.PictureBox1.Size = New System.Drawing.Size(40, 40)
    Me.PictureBox1.TabIndex = 27
    Me.PictureBox1.TabStop = False
    '
    'chkPrintReportParameters
    '
    Me.chkPrintReportParameters.Location = New System.Drawing.Point(108, 200)
    Me.chkPrintReportParameters.Name = "chkPrintReportParameters"
    Me.chkPrintReportParameters.Size = New System.Drawing.Size(228, 24)
    Me.chkPrintReportParameters.TabIndex = 28
    '
    'frmPrintBox
    '
    Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
    Me.CancelButton = Me.btnCancel
    Me.ClientSize = New System.Drawing.Size(350, 296)
    Me.Controls.Add(Me.chkPrintReportParameters)
    Me.Controls.Add(Me.PictureBox1)
    Me.Controls.Add(Me.lblPath)
    Me.Controls.Add(Me.lblPrintLocation)
    Me.Controls.Add(Me.grpBottomLine)
    Me.Controls.Add(Me.btnSetPrinter)
    Me.Controls.Add(Me.btnCancel)
    Me.Controls.Add(Me.btnOk)
    Me.Controls.Add(Me.grpPrintFormat)
    Me.Controls.Add(Me.txtCopies)
    Me.Controls.Add(Me.txtFileLocation)
    Me.Controls.Add(Me.lblCopies)
    Me.Controls.Add(Me.grpPrintOptions)
    Me.Controls.Add(Me.lblTitle)
    Me.Controls.Add(Me.grpTopLine)
    Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog
    Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
    Me.KeyPreview = True
    Me.MaximizeBox = False
    Me.MinimizeBox = False
    Me.Name = "frmPrintBox"
    Me.ShowInTaskbar = False
    Me.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent
    Me.grpPrintOptions.ResumeLayout(False)
    Me.grpPrintFormat.ResumeLayout(False)
    Me.grpBottomLine.ResumeLayout(False)
    Me.ResumeLayout(False)

  End Sub

#End Region

  Friend Function DisplayDialog() As String
    ' ********************************************************
    ' * Procedure:    DisplayDialog
    ' *
    ' * Description:  Displays the Form.  Returns any errors found.
    ' *
    ' * Parameters:   None
    ' *
    ' * Returns:      String - Blank=No Errors, Else Errors
    ' *
    ' * Date Created: 02/25/2003 by Peter Ringering
    ' ********************************************************
    m_bInProcess = True

    LoadResources()

    ShowDialog()

    Return m_strErrorXML

  End Function

  Friend Function DisplayProcess() As String
    ' ********************************************************
    ' * Procedure:    DisplayProcess
    ' *
    ' * Description:  Displays the Form.  Returns any errors found.
    ' *
    ' * Parameters:   None
    ' *
    ' * Returns:      String - Blank=No Errors, Else Errors
    ' *
    ' * Date Created: 02/25/2003 by Peter Ringering
    ' ********************************************************
    LoadResources()
    Me.lblTitle.Text = "Processing Report..."
    Me.lblTitle.TextAlign = ContentAlignment.TopCenter
    Me.lblTitle.Left() = 0
    Me.Text = "Cougar Mountain Software"
    Me.grpTopLine.Hide()
    Me.grpPrintOptions.Hide()
    Me.lblExportType.Hide()
    Me.rdoFile.Hide()
    Me.rdoScreen.Hide()
    Me.rdoPrinter.Hide()
    Me.grpPrintFormat.Hide()
    Me.rdoDot.Hide()
    Me.rdoLaser.Hide()
    Me.txtCopies.Hide()
    Me.lblCopies.Hide()
    Me.lblPath.Hide()
    Me.lblPrintLocation.Hide()
    Me.grpBottomLine.Hide()
    Me.GroupBox2.Hide()
    Me.btnSetPrinter.Hide()
    Me.btnCancel.Hide()
    Me.PictureBox1.Hide()
    Me.ControlBox() = False
    Me.Height = 70
    Me.Width = 294
    Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow

    ShowDialog()

    Return m_strErrorXML

  End Function

  Protected Overrides Sub OnGotFocus(
      ByVal e As EventArgs)

    If Not m_bInProcess Then
      m_bInProcess = True
      btnOk.PerformClick()
      Me.Close()
    End If

  End Sub

  Friend Sub SetPrintBoxOptions(
      ByVal strXMLPrintPrams As String)
    '****************************************************************/
    '	PrintBox_Load - Used to load print box dialog
    '
    '	Called - from .net dll(LoadPrintBox)
    '    Report Type (no editing allowed):
    '           Reports – all reports except Posting Audits. (Does
    '               this include Reprint Posting Reports?)
    '           Posting – Posting Audit Reports (only at time of
    '               posting – not reprints?)
    'Output Type (drop-down list box with 3 options):
    '           Printer (default)
    '           Screen
    '           File
    'Number of Copies (text field):
    '           Numeric, integers only, maximum = 99 (same as in current product)
    '           Only active if user selected Printer in Output Type field
    'Format (drop-down list box with 2 options):
    '           Controls format of report regardless of output destination:
    '           Laser (default)
    '           Dot Matrix
    'Printer/File Name:
    '           If Printer was selected as the Output Type, this is a
    '           drop-down list box showing the available printers. If
    '           File was selected as the Output Type, the default for
    '           this field is REPORT.TXT, just like in current product,
    '           but if File was selected as the Output Type,  the user
    '           will be able to type in the default location for the
    '           file and that can be modified, but NOT selected.  User
    '           must type in location.  Quote from PD “Changes”
    '           document:  “No dropdown will be shown for the file path
    '             if the file is the selected output type. This path will
    '             be entered by the user and validated to see if the path
    '             exists.  If it does not the user will be given a prompt
    '             with a choice to keep the path or change it to the
    '             default path of <data path>\report.txt."
    'Edit (check box):
    '           If this check box is selected then when the type of report
    '           on the selected detail line (“Reports” vs. “Posting”) is
    '           printed, the user will see the same Print Options dialog
    '           as in the current application, and can edit the output
    '           selections just as they can in the current application.
    '           However, the defaults set on that Print Options Dialog
    '           would be what the user set on this screen. If this check
    '           box is not selected, the system will print the selected
    '           report using the options here, without displaying the
    '           Print Options dialog box.
    '
    '****************************************************************/

    Dim DocXml As New XmlDocument   'xml dox
    Dim eleLang As XmlElement
    'Dim eleLangPrams As XmlElement 'XML element
    'Dim strFileFormat As String

    SetExportObjects(False) 'Set the export controls
    DocXml.LoadXml(strXMLPrintPrams) 'set XMLstring in a doc
    eleLang = DocXml.DocumentElement("REPORTPARAMS") 'get elements from XML AES
    If Not IsNothing(eleLang) Then
      m_intOUTPUTTYPE = CInt(eleLang.SelectSingleNode("OUTPUTTYPE").InnerXml)
      m_strNUMBEROFCOPIES = eleLang.SelectSingleNode("COPIES").InnerXml
      m_intFORMAT = CInt(eleLang.SelectSingleNode("FORMAT").InnerXml)
      'Defect ID 1-14261, PGP(11/23/2004) - Changed to InnerText so &'s work right.
      m_strPRINTER = eleLang.SelectSingleNode("PRINTER").InnerText
      If (m_strPRINTER.Length() = 0) Then
        m_strPRINTER = g_IDS_DEFAULT
      End If
      m_bEdit = (eleLang.SelectSingleNode("EDIT").InnerXml = "1")
      'Added for print report summary AES 6.15.04
      m_bPRINT_REPORT_PARAMETERS = (eleLang.SelectSingleNode("PRINT_REPORT_PARAMETERS").InnerXml = "1")

      m_nReportType = CType([Enum].Parse(m_nReportType.GetType, eleLang.SelectSingleNode("REPORTTYPE").InnerXml, True), enuReportType)
      ''changed for an if statment 6.16.04 AES
      Select Case m_nReportType                     'do the layout for the report type.
        Case enuReportType.RT_Report
          chkPrintReportParameters.Show()

        Case enuReportType.RT_Statements
          Me.rdoDot.Show()
          Me.rdoDot.Enabled = True
          Me.grpPrintFormat.Show()
          Me.grpPrintFormat.Enabled = True
          Me.rdoLaser.Show()
          Me.rdoLaser.Enabled = True

        Case Else
          chkPrintReportParameters.Hide()

      End Select
      Me.lblPrintLocation.Text = m_strPRINTER 'path of the printer
      'Peter Ringering - 05/25/2004 - 1-9876
      If Not IsNothing(eleLang("FILENAME")) Then
        Me.txtFileLocation.Text = eleLang.SelectSingleNode("FILENAME").InnerXml()
      End If
      'set the output type on form
      Select Case m_intOUTPUTTYPE
        Case enuOutputType.OT_Printer
          Me.rdoPrinter.Checked = True 'set the rdo button default to Print
          Me.m_nGrpOption = enuOutputType.OT_Printer 'set grp variable to match rdo button. AES
          Me.rdoPrinter.Focus() 'set the focus to the Print rdo AES
          Me.txtFileLocation.Enabled = False
          Me.rdoPrinter.Select()
        Case enuOutputType.OT_Screen
          Me.rdoScreen.Checked = True 'set the rdo button default to screen
          Me.m_nGrpOption = enuOutputType.OT_Screen 'set grp variable to match rdo button. AES
          Me.txtFileLocation.Enabled = False
          Me.rdoScreen.Select()
        Case enuOutputType.OT_File
          Me.rdoFile.Checked = True 'set the rdo button default to Print
          Me.m_nGrpOption = enuOutputType.OT_File 'set grp variable to match rdo button. AES
          Me.rdoFile.Focus() 'set the focus to the Print rdo AES
          Me.rdoFile.Select()
          Me.txtFileLocation.Enabled = True
      End Select
      'set the format type on form
      Select Case m_intFORMAT
        Case enuFormatType.FT_Dot
          Me.rdoDot.Checked = True
          Me.m_nGrpPrintType = 2
        Case enuFormatType.FT_Laser
          Me.rdoLaser.Checked = True 'set rdo button for laser
          Me.m_nGrpPrintType = 1 'set the print type here AES                    'set the focus to the screen rdo AES
      End Select
      ''set the number of copies from the prams
      Me.txtCopies.Text = m_strNUMBEROFCOPIES    'number of copies
      Me.chkPrintReportParameters.Checked() = m_bPRINT_REPORT_PARAMETERS      'if you want to print report summarys
    Else 'set the default value because no printer prams were passed in
      '*********************this code need to have the functionality of the 16 bit for setting up printers
      'here is were the you will look at the variables that a set for this AES
      Me.rdoScreen.Checked = True 'set the rdo button default to screen
      Me.m_nGrpOption = 2 'set grp variable to match rdo button. AES
      Me.rdoScreen.Focus() 'set the focus to the screen rdo AES
      Me.rdoLaser.Checked = True 'set rdo button for laser
      Me.m_nGrpPrintType = 1 'set the print type here AES
      '*************************************************************************
      Me.txtCopies.Text = "1" 'number of copies
      Me.m_strCopies = txtCopies.Text

      Me.rdoFile.Enabled = False
    End If

  End Sub

  Friend Sub LoadResources()
    '****************************************************************/
    '	LoadResources - Used to load lang file
    '
    '	Called -	    load form
    '****************************************************************/

    Dim strFileFormat As String

    'labels
    'MsgBox("Load Resources")
    If Me.Text = String.Empty Then
      Me.Text = g_IDS_REPORTPRINTOPTIONS
    End If
    'Peter Ringering - 03/27/2003 - 1-3977
    Dim objXMLDoc As New XmlDocument
    Try
      objXMLDoc.LoadXml(m_strXMLParameters)
      Dim objReport As XmlElement = objXMLDoc.DocumentElement("REPORT") 'get elements from XML AES
      If Not (IsNothing(objReport)) Then
        If Not objReport("REPORTTITLE") Is Nothing Then
          '// DG, 3/28/2003, Defect 1-4057, Set new property.
          Me.ReportTitle = Trim(objReport("REPORTTITLE").InnerText)
          Me.Text = Trim(objReport("REPORTTITLE").InnerText) & " " & g_IDS_REPORTPRINTOPTIONS
        End If
      End If
    Catch
    End Try

    Me.lblTitle.Text = g_IDS_PRINTOPTIONDESC
    Me.lblCopies.Text = g_IDS_NOOFCOPIES
    Me.lblExportType.Text = g_IDS_EXPORTFORMAT
    'Peter Ringering - 05/25/2004 - 1-9876
    If (Trim(txtFileLocation.Text) = String.Empty) Then
      Me.txtFileLocation.Text = g_IDS_EXPORT_FILENAME
    End If
    'buttons
    Me.btnCancel.Text = g_IDS_CANCEL
    Me.btnOk.Text = g_IDS_OK
    Me.btnSetPrinter.Text = g_IDS_SETUPPRINTER
    'radio buttons
    Me.rdoFile.Text = g_IDS_FILE
    Me.rdoPrinter.Text = g_IDS_PRINTER
    Me.rdoScreen.Text = g_IDS_SCREEN
    'radio for the format
    Me.rdoDot.Text = g_IDS_DOTMATRIX
    Me.rdoLaser.Text = g_IDS_LASERPRINTER
    'current path to printer
    Me.lblPath.Text = g_IDS_CURPRINTER
    'set the combobox for export options
    Me.cboExportType.Items.Insert(enuExportFormat.WORD, g_IDS_WORDFORMAT)
    Me.cboExportType.Items.Insert(enuExportFormat.EXCEL, g_IDS_EXCELFORMAT)
    Me.cboExportType.Items.Insert(enuExportFormat.PDF, g_IDS_PDFFORMAT)
    Me.cboExportType.Items.Insert(enuExportFormat.HTML40, g_IDS_HTML40FORMAT)
    Me.cboExportType.Items.Insert(enuExportFormat.XML, g_IDS_XMLFORMAT)
    'added for print report summary AES 6.16.04
    Me.chkPrintReportParameters.Text = g_IDS_PRINT_REPORT_PARAMETERS

    'add to select the file type in combobox AES
    m_strFileName = Me.txtFileLocation.Text
    strFileFormat = m_strFileName.Substring(m_strFileName.Length - 4)
    Select Case strFileFormat
      Case ".doc", ".DOC"
        cboExportType.SelectedIndex = enuExportFormat.WORD
      Case ".xls", ".XLS"
        cboExportType.SelectedIndex = enuExportFormat.EXCEL
      Case ".pdf", ".PDF"
        cboExportType.SelectedIndex = enuExportFormat.PDF
      Case "html", "HTML"
        cboExportType.SelectedIndex = enuExportFormat.HTML40
      Case ".xml", ".XML"
        cboExportType.SelectedIndex = enuExportFormat.XML
      Case Else
        Me.cboExportType.SelectedIndex = enuExportFormat.WORD
    End Select

  End Sub

  Friend Function RunAll() As String
    '****************************************************************/
    '	RunAll - OK button action, run report
    '
    '	Called -	    from .net dll(PrintBox)
    '****************************************************************/

    If Not g_bRunClientData Then 'Check to see if the report data is client or server
      g_bVal = RunReport() 'Use server data
    End If

    If g_bVal Then   'check is the report functions passed

      Dim pd As New PrintDocument

      'If this is a file export you do not need to val printer path. AES error in beta build 1
      If (m_nGrpOption <> enuOutputType.OT_File) Then
        ' Erick Korsten - 5/30/3 - Defect-ID 1-4463 - Set Printer name
        If Me.lblPrintLocation.Text = String.Empty Or IsNothing(Me.lblPrintLocation.Text) Then
          Me.lblPrintLocation.Text = "Default"
        End If
        If Me.lblPrintLocation.Text <> "Default" Then
          Dim strPrinter As String = Me.lblPrintLocation.Text
          strPrinter.Replace("(LPT:)", "")
          strPrinter.Replace("(LPT)", "")
          strPrinter.Replace("(lpt:)", "")
          strPrinter.Replace("(lpt)", "")
          m_objReport.PrintOptions.PrinterName = strPrinter.Trim()
          pd.PrinterSettings.PrinterName = strPrinter.Trim()
        End If
      End If

      Select Case m_nGrpOption    'Check which print option is chosen aES
        Case enuOutputType.OT_Printer
          Me.m_strCopies = txtCopies.Text
          If (pd.PrinterSettings.IsValid) Then
            m_objReport.PrintToPrinter(CInt(m_strNUMBEROFCOPIES), True, 0, 0)      'send the report to the printer AES
          Else
            MsgBox(g_IDS_PRINTERINVALID, MsgBoxStyle.Information, g_IDS_REPORTPRINTOPTIONS)
          End If

          Me.Close()

        Case enuOutputType.OT_Screen
          'Me.SendToBack()    ' Erick Korsten ~ 08/23/04 ~ This call sends the CMS application to the end of the app. list.
          Dim frmReportViewer As New frmReportViewer
          frmReportViewer.RemoveOwnedForm(Me)
          frmReportViewer.SetReport(m_objReport)      'Send the report to the viewer AES
          frmReportViewer.Text = m_strReportTitle
          m_hwPrintScreen = frmReportViewer.Handle.ToInt32

          'This is important.  We need to make this top most so the print options dialog won't show
          'after the print screen shows.  The print options dialog will set this property to false
          'when it gets focus.
          frmReportViewer.TopMost = True

          Application.DoEvents()
          frmReportViewer = Nothing

        Case enuOutputType.OT_File
          CMSExport()

      End Select
    End If

    '    Me.Cursor = Cursors.Default   'defalut cursor
    System.GC.Collect()   ''cleanup

    Return m_strErrorXML

  End Function

  Friend Function RunReportFromClientData(
      ByRef p_dat As DataSet,
      ByRef p_strCrystalParams As String) As Boolean

    Dim objWaitThread As New WaitThread   'wait thread class
    Dim thdWait As New Thread(AddressOf objWaitThread.WaitForm)
    Dim objReportThread As New ReportThread   'report thread class
    objReportThread.m_strReportTitle = m_strReportTitle
    Dim thdReport As New Thread(AddressOf objReportThread.LoadReport)

    g_intHeaderRecordCount = 0
    g_intDetailRecordCount = 0

    Try
      Dim DocXml As New XmlDocument

      m_strErrorXML = String.Empty

      ''Developer view used for troubleshooting errors
      If m_bDeveloperViewer Then
        Dim frmXMLDotNetViewer As New frmXMLDotNetViewer   'Initialize Viewer
        frmXMLDotNetViewer.txtReportXMLText.Text = p_strCrystalParams 'set pass in XML to windows
        frmXMLDotNetViewer.txtSeverComXMLText.Text = String.Empty
        frmXMLDotNetViewer.txtSeverDataXMLText.Text = p_dat.GetXml
        frmXMLDotNetViewer.ShowDialog()
        If frmXMLDotNetViewer.bCancel Then 'cancel was pushed dont show report
          objWaitThread.bCancel = True
          Return False
          Me.Close()
        End If
        m_strCommand = frmXMLDotNetViewer.txtSeverComXMLText.Text 'if values change in viewer reset them
        m_strData = frmXMLDotNetViewer.txtSeverDataXMLText.Text
        m_strXMLParameters = frmXMLDotNetViewer.txtReportXMLText.Text
      End If

      objReportThread.m_strXMLParameters = p_strCrystalParams 'set parameter string
      thdReport.Start() 'start report thread
      Application.DoEvents()

      p_dat.DataSetName = "XMLDataSet" 'name data set
      Do While objReportThread.bDoneLoading = False 'Check to see if the report theard is complete
        Application.DoEvents()
      Loop
      m_objReport = objReportThread.m_objReport 'set report to report from thread

      'Get the dataset record count and put it into a global
      If p_dat.Tables.Count >= 1 Then
        g_intHeaderRecordCount = p_dat.Tables(0).Rows.Count
      End If
      objWaitThread.mode = WaitThread.enuMode.MD_PROCESSDATA 'wait thread mode

      If p_dat.Tables.Count >= 2 Then
        g_intDetailRecordCount = p_dat.Tables(1).Rows.Count
      End If

      'This is for testing
      'Dim rdr As New IO.StringWriter()
      'p_dat.WriteXml(rdr, XmlWriteMode.WriteSchema)
      'MsgBox(rdr.ToString)
      m_XMLDataset = p_dat 'DAJ 3/30/5 err: 1-15823 gotta set this member variable or XML files won't have data to print
      m_objReport.SetDataSource(p_dat) 'set reports data source

      ' Erick Korsten ~ 09/08/04 ~ Set data source for subreports
      SetSubReportDataSource(m_objReport, p_dat)

      objWaitThread.bCancel = True

    Catch err As System.Exception 'catch com errors
      objWaitThread.bCancel = True 'stop wait thread
      Application.DoEvents()
      MsgBox(err.Message)
      '      Me.Cursor = Cursors.Default 'end cursor
      Return False
      Me.Close()
      'Add any initialization after the InitializeComponent() call
    End Try

    Return True

  End Function

  Private Sub CMSExport()

    'get the export options.
    Dim exportOpts As New ExportOptions
    Dim diskOpts As New DiskFileDestinationOptions      'disk file destination optons
    Dim htmlOpts As New HTMLFormatOptions
    Dim FormatWordOpts As New PdfRtfWordFormatOptions   'pdf format
    Dim FormatExeclOpts As New ExcelFormatOptions       'excel format
    Dim FormatPDFOpts As New PdfRtfWordFormatOptions    'word format
    Dim FormatHTML40Opts As New HTMLFormatOptions       'html format
    Dim szFormatEx As String = ".doc"                   'string for file Extension
    Dim szDiskFileName As String = txtFileLocation.Text
    Dim nCount, nLength As Integer
    'Dim strTest As String
    Dim strType As String
    Dim bolAppendSuffix As Boolean = False
    Dim intSelectedIndex As Integer = 0
    exportOpts = m_objReport.ExportOptions                     'set report for export

    'If cboExportType.SelectedIndex < 0 Then
    'strType = m_strPRINTER.Substring(m_strPRINTER.Length - 4)
    If m_strFileName.IndexOf(".") > 0 Then
      strType = m_strFileName.Substring(m_strFileName.Length - 4)
      Select Case strType
        Case "html", "HTML"
          bolAppendSuffix = True
          intSelectedIndex = enuExportFormat.HTML40

        Case ".xml", ".XML"
          bolAppendSuffix = True
          intSelectedIndex = enuExportFormat.XML

        Case ".doc", ".DOC"
          bolAppendSuffix = True
          intSelectedIndex = enuExportFormat.WORD

        Case ".xls", ".XLS"
          bolAppendSuffix = True
          intSelectedIndex = enuExportFormat.EXCEL

        Case ".pdf", ".PDF"
          bolAppendSuffix = True
          intSelectedIndex = enuExportFormat.PDF

      End Select
      If Not bolAppendSuffix Then
        MsgBox(strType & " is not a valid export type file format.", MsgBoxStyle.Information, g_IDS_REPORTPRINTOPTIONS)
        Exit Sub
      End If
    Else
      intSelectedIndex = cboExportType.SelectedIndex
    End If
    Select Case intSelectedIndex 'get current selection from combobox
      Case enuExportFormat.WORD 'word export format
        exportOpts.FormatOptions = FormatWordOpts
        exportOpts.ExportFormatType = ExportFormatType.WordForWindows
        szFormatEx = ".doc"

      Case enuExportFormat.EXCEL 'excel export format
        exportOpts.FormatOptions = FormatExeclOpts
        exportOpts.ExportFormatType = ExportFormatType.Excel
        szFormatEx = ".xls"

      Case enuExportFormat.PDF 'PDF export format
        exportOpts.FormatOptions = FormatPDFOpts
        exportOpts.ExportFormatType = ExportFormatType.PortableDocFormat
        szFormatEx = ".pdf"

      Case enuExportFormat.HTML40
        szFormatEx = ".html"
        '1-8096 AES not correct location for html
        m_strReportName = m_objReport.FilePath()
        m_strReportName = m_strReportName.Substring(0, m_strReportName.Length - 4)
        For nLength = m_strReportName.Length To 0 Step -1
          If (m_strReportName.Substring(nLength - 1, 1) = "\") Then
            m_strReportName = m_strReportName.Substring(nLength, nCount)
            Exit For
          End If
          nCount += 1
        Next nLength
        txtFileLocation.Text = txtFileLocation.Text.Replace(m_strFileName, m_strReportName & "\" & m_strFileName)
        htmlOpts.HTMLFileName = m_strFileName
        htmlOpts.HTMLBaseFolderName = m_strFilePath
        htmlOpts.HTMLEnableSeparatedPages = True
        htmlOpts.HTMLHasPageNavigator = True
        exportOpts.FormatOptions = htmlOpts 'html format
        exportOpts.ExportFormatType = ExportFormatType.HTML40

      Case enuExportFormat.XML 'xml format
        szFormatEx = ".xml"
        m_XMLDataset.WriteXml(m_szDiskFileName, XmlWriteMode.WriteSchema)

    End Select

    If intSelectedIndex <> enuExportFormat.XML Then
      exportOpts.ExportDestinationType = ExportDestinationType.DiskFile 'export destination type
      ' Set the disk file options.
      If IsNothing(m_szDiskFileName) Then
        diskOpts.DiskFileName = Application.StartupPath & "\" & m_strPRINTER   'set where and the name of the export file
      Else
        diskOpts.DiskFileName = m_szDiskFileName    'set where and the name of the export file
      End If
      exportOpts.DestinationOptions = diskOpts
      m_objReport.Export()
    End If

    'export file
    'clean up the objects
    exportOpts = Nothing
    diskOpts = Nothing
    FormatWordOpts = Nothing
    FormatExeclOpts = Nothing
    FormatPDFOpts = Nothing
    FormatHTML40Opts = Nothing

    Me.Close() 'close form after export
    'send the report to the file export AES

  End Sub

  Private Sub btnOk_Click(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles btnOk.Click
    '****************************************************************/
    '	btnOk_Click - OK button action, run report
    '
    '	Called -	    from .net dll(PrintBox)
    '****************************************************************/

    Me.Cursor = Cursors.WaitCursor 'wait cursor
    Try
      'Defect 1-4136, 6/10/2003, Ken Moss -- Validate the path
      Dim bolFailed As Boolean = True
      Dim szReportName As String = m_objReport.FilePath()
      If m_bInProcess Then
        txtFileLocation.Focus()
      End If

      Dim strFile As String = txtFileLocation.Text.Substring(0, 3)
      If rdoFile.Checked Then
        'Append the extension to the end of the file
        Dim strSuffix As String = String.Empty
        Dim str5LetterSuffix As String = String.Empty
        If txtFileLocation.Text.Length > 4 Then strSuffix = (txtFileLocation.Text.Substring(txtFileLocation.Text.Length - 4, 4))
        If txtFileLocation.Text.Length > 5 Then str5LetterSuffix = (txtFileLocation.Text.Substring(txtFileLocation.Text.Length - 5, 5))

        Dim bolAppendSuffix As Boolean = True
        Select Case strSuffix
          Case "html", "HTML"
            If str5LetterSuffix.ToLower = ".html" Then
              bolAppendSuffix = False
            End If

          Case ".xml", ".doc", ".xls", ".pdf", ".XML", ".DOC", ".XLS", ".PDF"
            bolAppendSuffix = False

        End Select
        If bolAppendSuffix Then
          strSuffix = cboExportType.Text
          strSuffix = strSuffix.Substring(strSuffix.IndexOf("*.") + 1, 4)
          'Handle the html instance, it has 4 letters
          If (strSuffix = ".htm") Then strSuffix = ".html"
          txtFileLocation.Text += strSuffix
        End If
        Dim fil As New FileInfo(txtFileLocation.Text)
        Dim di As New DirectoryInfo(strFile)
        'See if the text they put in exists
        '1-8096 AES not correct location for html
        'If Not (fil.Directory.Exists) Then
        If di.Exists Then
          bolFailed = False
          m_strFileName = fil.Name
          '1-8096 AES not correct location for html
          m_strFilePath = fil.Directory.FullName
          txtFileLocation.Text = fil.FullName
          m_szDiskFileName = txtFileLocation.Text
          If m_strFileName.Substring(0, 1) = "." Then
            MsgBox("The File Specified Is Not Valid (" & fil.Name & ").", MsgBoxStyle.OkOnly, "Specified File Name Invalid")
            bolFailed = True
          End If
        Else
          Try
            'If this fails then see if they were expecting to append the file name Application.StartupPath & "\"
            'on the application path.
            Dim strAppPath As String = Application.StartupPath
            strAppPath.Trim()
            'Append a separator in front of the name of the file
            If txtFileLocation.Text.Substring(0, 1) <> "\" Then strAppPath += "\"

            fil = New IO.FileInfo(strAppPath & txtFileLocation.Text)
            If Not fil.Directory.Exists Then
              'MsgBox("The path specified does not exist!  Please enter a valid path.", MsgBoxStyle.OKOnly, "Specified Path Invalid")
              Throw New Exception
            Else
              m_strFileName = fil.Name
              '1-8096 AES not correct location for html
              m_strFilePath = fil.Directory.FullName
              txtFileLocation.Text = fil.FullName
              m_szDiskFileName = txtFileLocation.Text
              Application.DoEvents()
              bolFailed = False
            End If
          Catch ex As Exception
            MsgBox("The path specified does not exist!  Please enter a valid path.", MsgBoxStyle.OkOnly, "Specified Path Invalid")
          End Try

        End If
        'Make sure the file is released and the garbage collector also
        'releases the handle
        fil = Nothing
        GC.Collect()
      Else
        bolFailed = False
      End If
      Application.DoEvents()
      If Not bolFailed Then RunAll() 'run the reports
    Catch ex As Exception
      MsgBox(ex.Message)
    End Try

    Me.Cursor = Cursors.Default

  End Sub

  Private Sub btnCancel_Click(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles btnCancel.Click
    '****************************************************************/
    '	btnCancel_Click - OK button action, close dialog
    '
    '	Called -	    from .net dll(PrintBox)
    '****************************************************************/

    Me.Close()

  End Sub

  Friend Function RunReport() As Boolean
    '****************************************************************/
    '	RunReport - Run Report for printing
    '
    '	Called -	    from .net dll(btnOk_Click)
    '****************************************************************/

    Dim objWaitThread As New WaitThread     'wait thread class
    Dim thdWait As New Thread(AddressOf objWaitThread.WaitForm)
    Dim XMLDataset As New DataSet
    Dim objReportThread As New ReportThread     'report thread class
    objReportThread.m_strReportTitle = m_strReportTitle
    Dim thdReport As New Thread(AddressOf objReportThread.LoadReport)

    g_intHeaderRecordCount = 0
    g_intDetailRecordCount = 0

    Try
      Dim xmlstring As String
      Dim DocXml As New XmlDocument

      ''Developer view used for troubleshooting errors
      If m_bDeveloperViewer Then
        Dim frmXMLDotNetViewer As New frmXMLDotNetViewer        'Initialize Viewer
        frmXMLDotNetViewer.txtReportXMLText.Text = m_strXMLParameters    'set pass in XML to windows
        frmXMLDotNetViewer.txtSeverComXMLText.Text = m_strCommand
        frmXMLDotNetViewer.txtSeverDataXMLText.Text = m_strData
        frmXMLDotNetViewer.ShowDialog()
        If frmXMLDotNetViewer.bCancel Then                      'cancel was pushed dont show report
          objWaitThread.bCancel = True
          Return False
          Me.Close()
        End If
        m_strCommand = frmXMLDotNetViewer.txtSeverComXMLText.Text     'if values change in viewer reset them
        m_strData = frmXMLDotNetViewer.txtSeverDataXMLText.Text
        m_strXMLParameters = frmXMLDotNetViewer.txtReportXMLText.Text
      End If
      'report thread paramater string
      objReportThread.m_bDotMatrix = Me.rdoDot.Checked
      objReportThread.m_strXMLParameters = m_strXMLParameters                   'set parameter string
      objReportThread.m_bPRINT_REPORT_PARAMETERS = chkPrintReportParameters.Checked()     'add for report summary printing AES
      thdReport.Start()    'start report thread
      Application.DoEvents()

      'Get the data that supposed to be going to the report.
      xmlstring = UTL_TransactedServer.DoXMLCommand(m_strCommand, m_strData)

      Debug.WriteLine(vbCrLf + xmlstring + vbCrLf)

      If InStr(xmlstring, "<ERROR>") > 0 Then                       'check for an error
        objWaitThread.bCancel = True
        Application.DoEvents()                                  'end wait thread
        m_strErrorXML = xmlstring
        Me.Close()     'close print
        Return False
      ElseIf InStr(xmlstring, "<NODATA>") > 0 Then                  'check for no data AES
        objWaitThread.bCancel = True
        Application.DoEvents()                                  'end wait thread
        MsgBox("No information matches the selected report settings. There is nothing to report.", MsgBoxStyle.Exclamation, m_strReportTitle)    'show msg 'hae
        Me.Close()     'close print
        Return False
      End If
      Dim rdrText As New StringReader(xmlstring)    'set retrun string to xml string and text reader
      Dim rdrXML As New XmlTextReader(rdrText)
      If Not IsNothing(XMLDataset) Then    'clear dataset if this is the report was printed before AES
        XMLDataset.Clear()
      End If
      XMLDataset.ReadXml(rdrXML)    'set text to dataset
      Application.DoEvents()
      XMLDataset.DataSetName = "XMLDataSet"    'name data set
      Do While objReportThread.bDoneLoading = False    'Check to see if the report theard is complete
        Application.DoEvents()
      Loop
      If Not objReportThread.m_bError Then
        m_objReport = objReportThread.m_objReport    'set report to report from thread
      Else
        m_objReport = Nothing
        Return False
      End If
      'Get the dataset record count and put it into a global
      If XMLDataset.Tables.Count >= 1 Then
        g_intHeaderRecordCount = XMLDataset.Tables(0).Rows.Count
      End If
      objWaitThread.mode = WaitThread.enuMode.MD_PROCESSDATA    'wait thread mode
      If XMLDataset.Tables.Count >= 2 Then
        g_intDetailRecordCount = XMLDataset.Tables(1).Rows.Count
      End If
      m_XMLDataset = XMLDataset    'set variable for xml export
      m_objReport.SetDataSource(XMLDataset)    'set reports data source

      ' Erick Korsten ~ 09/08/04 ~ Set data source for subreports
      SetSubReportDataSource(m_objReport, XMLDataset)

      objWaitThread.bCancel = True

    Catch err As Exception   'catch com errors
      objWaitThread.bCancel = True    'stop wait thread
      Application.DoEvents()
      MsgBox(err.Message)
      '      Me.Cursor = Cursors.Default    'end cursor
      Return False
      Me.Close()
      'Add any initialization after the InitializeComponent() call
    End Try

    Return True

  End Function

  Private Sub rdoPrinter_CheckedChanged(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles rdoPrinter.CheckedChanged
    '****************************************************************
    '	rdoPrinter_CheckedChanged -  Used to set m_nGrpOption to 1
    '                       for print to screen
    '	Called -	    from .net dll(PrintBox)
    '****************************************************************

    m_nGrpOption = enuOutputType.OT_Printer   'Use printer as the output for the report AES
    Me.txtFileLocation.Enabled = False
    Me.txtCopies.Enabled = True
    SetExportObjects(False)

  End Sub

  Private Sub rdoScreen_CheckedChanged(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles rdoScreen.CheckedChanged
    '****************************************************************/
    '	rdoScreen_CheckedChanged -  Used to set m_nGrpOption to 2
    '                       for print to printer
    '	Called -	    from .net dll(PrintBox)
    '****************************************************************/

    m_nGrpOption = enuOutputType.OT_Screen   'Use screen as the output for the report AES
    Me.txtFileLocation.Enabled = False
    Me.txtCopies.Enabled = False
    SetExportObjects(False)

  End Sub

  Private Sub rdoFile_CheckedChanged(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles rdoFile.CheckedChanged
    '****************************************************************/
    '	rdoFile_CheckedChanged -  Used to set m_nGrpOption to 3
    '                       for export to file
    '	Called -	    from .net dll(PrintBox)
    '****************************************************************/

    m_nGrpOption = enuOutputType.OT_File   'Use file export as the output for the report AES
    Me.txtFileLocation.Enabled = True
    Me.txtCopies.Enabled = False
    SetExportObjects(True)

  End Sub

  Private Sub SetExportObjects(
      ByVal bSetValue As Boolean)
    '****************************************************************/
    '	SetExportObjects - Change the state of export objects
    '
    '	Called -	 frmPrintBox to change the set of the export
    '                objects.
    '****************************************************************/

    If rdoFile.Checked Then   'check for state
      SetExportVisable(True)    'set if the objects should be visable
    Else
      SetExportVisable(False)
    End If
    Me.lblExportType.Enabled = bSetValue
    Me.cboExportType.Enabled = bSetValue

  End Sub

  Private Sub SetExportVisable(
      ByVal bSetValue As Boolean)
    '****************************************************************/
    '	SetExportVisable - Change the state of the form and export
    '                      object
    '
    '	Called -	frmPrintBox when the form loads and exprot options
    '               is clicked
    '****************************************************************/

    Dim intGroupHeight As Integer

    If bSetValue Then    'Set the state for export options visable
      intGroupHeight = 70    ' move form and object
      Me.txtFileLocation.SetBounds(6, 137, 336, 20)    '
      Me.grpPrintFormat.SetBounds(145, 163, 198, 35)    '
      Me.lblCopies.SetBounds(6, 174, 96, 20)    '
      Me.txtCopies.SetBounds(103, 174, 32, 20)    '
      Me.chkPrintReportParameters.SetBounds(150, 177, 200, 16)    '
      Me.grpBottomLine.SetBounds(6, 224, 336, 7)    '
      Me.lblPath.SetBounds(6, 205, 96, 16)    '
      Me.lblPrintLocation.SetBounds(103, 205, 240, 16)    '
      Me.btnCancel.SetBounds(161, 240, 87, 26)    '
      Me.btnOk.SetBounds(69, 240, 87, 26)    '
      Me.btnSetPrinter.SetBounds(253, 240, 87, 26)    '
    Else    ' Set the state for export options not visable
      intGroupHeight = 40    ' move form and object
      Me.txtFileLocation.SetBounds(6, 104, 336, 20)    '
      Me.grpPrintFormat.SetBounds(145, 128, 198, 35)    '
      Me.lblCopies.SetBounds(6, 139, 96, 20)    '
      Me.txtCopies.SetBounds(103, 139, 32, 20)    '
      Me.chkPrintReportParameters.SetBounds(150, 142, 200, 16)    '
      Me.lblPath.SetBounds(6, 170, 96, 16)    '
      Me.lblPrintLocation.SetBounds(103, 170, 240, 16)    '
      Me.grpBottomLine.SetBounds(6, 192, 336, 7)    '
      Me.btnCancel.SetBounds(161, 208, 87, 26)    '
      Me.btnOk.SetBounds(69, 208, 87, 26)    '
      Me.btnSetPrinter.SetBounds(253, 208, 87, 26)    '
    End If
    Me.Height = Me.btnOk.Top + Me.btnOk.Height + (Me.Height - Me.ClientRectangle.Height) + 4   '
    Me.lblExportType.Visible = bSetValue   ' set if the object should be visable
    Me.cboExportType.Visible = bSetValue
    Me.grpPrintOptions.Height = intGroupHeight

  End Sub

  Private Sub txtCopies_TextChanged(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles txtCopies.TextChanged

    m_strNUMBEROFCOPIES = Me.txtCopies.Text   ' set the value to the number in the box

  End Sub

  Private Sub btnSetPrinter_Click(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles btnSetPrinter.Click

    Dim pdlg As New PrintDialog
    Dim pset As New PrinterSettings

    pdlg.PrinterSettings = pset
    Me.Cursor = Cursors.WaitCursor
    pdlg.ShowDialog(Me)
    Me.lblPrintLocation.Text = pdlg.PrinterSettings.PrinterName
    '1-8757 AES 12.03.02 Change to add number of copies if it changes on setup form.
    Me.txtCopies.Text = pdlg.PrinterSettings.Copies.ToString
    Me.Cursor = Cursors.Default   'wait cursor

  End Sub

  Private Sub frmPrintBox_KeyPress(
      ByVal sender As Object,
      ByVal e As KeyPressEventArgs) _
      Handles MyBase.KeyPress

    If e.KeyChar = Microsoft.VisualBasic.ChrW(13) Then
      SendKeys.SendWait("{Tab}")
      e.Handled = True
    End If

  End Sub

  Private Sub frmPrintBox_Load(
      ByVal sender As Object,
      ByVal e As EventArgs) _
      Handles MyBase.Load

  End Sub

  Private Sub SetSubReportDataSource(
      ByRef p_objReport As ReportDocument,
      ByVal p_XMLDataset As DataSet)

    ' Set the datasource for all the Sub reports (if any)

    ' Erick Korsten ~ 09/08/04 ~ Loop through report objects and find subreport objects
    For Each oReportObject As ReportObject In p_objReport.ReportDefinition.ReportObjects

      If oReportObject.Kind = ReportObjectKind.SubreportObject Then
        ' This is a sub-report: set the datasource of sub report

        ' Cast the object as a Subreport object
        Dim oSubreport As SubreportObject = CType(oReportObject, SubreportObject)

        ' Open the subreport and set datasource
        Dim oSubreportDoc As ReportDocument = p_objReport.OpenSubreport(oSubreport.SubreportName)
        oSubreportDoc.SetDataSource(p_XMLDataset)
        Exit For
      End If
    Next

  End Sub

End Class

Friend Class ReportThread
  '****************************************************************/
  '	ReportThread -  Used to set up the report, runs the same time
  '                   data is being retrived from SQL
  '	Called -	    from .net dll(RunReport)
  '****************************************************************/

  Friend WithEvents m_objReport As New ReportDocument
  Friend m_strSubReportName As String = String.Empty
  Friend m_bError As Boolean = False
  Friend bDoneLoading As Boolean = False  'set report theard variables
  Friend m_ReportLocation As String = String.Empty
  Friend m_strXMLParameters As String = String.Empty
  Friend DocXml As Xml.XmlDocument      'XML doc
  Friend m_strReportTitle As String = String.Empty
  Friend m_bDotMatrix As Boolean = False
  Friend m_bPRINT_REPORT_PARAMETERS As Boolean = True

  Friend Sub LoadReport()

    Dim strReportName As String

    Dim eleReport As XmlNode
    Dim eleMatrix As XmlNode    'XML element

    Try
      DocXml = New XmlDocument
      DocXml.LoadXml(m_strXMLParameters)    'set XMLstring in a doc
      eleReport = DocXml.DocumentElement("REPORT")    'get report path from XML AES
      strReportName = eleReport("szReport").InnerText  'eleReportParams.InnerText

      'sb - 22-sep-2004 - 1-13538
      If Not eleReport.SelectSingleNode("bolDotMatrix") Is Nothing Then
        eleMatrix = eleReport.SelectSingleNode("bolDotMatrix")
        m_bDotMatrix = CBool(eleMatrix.InnerXml)
      End If

      If m_bDotMatrix Then
        'sb -  22-sep-2004 - 1-13538
        Debug.WriteLine(vbCrLf & "/*********************************************************************/")
        Debug.WriteLine("Dot Matrix: TRUE" & vbCrLf)
        Debug.WriteLine("/*********************************************************************/" & vbCrLf)
        strReportName = strReportName.Replace(".rpt", "DOT.rpt")
      End If

      Try
        m_objReport.Load(strReportName)    'load report form path aES
      Catch ex As Exception
        MsgBox(ex.Message)
      End Try

      '*********************************************************************
      If m_objReport.IsLoaded Then
        Debug.WriteLine(vbCrLf & "/*********************************************************************/")
        Debug.WriteLine("Report Name: " & m_strReportTitle)
        Debug.WriteLine("Report Path: " & m_objReport.FilePath)
        Debug.WriteLine("/*********************************************************************/" & vbCrLf)
      End If
      '*********************************************************************

      m_bError = LoadReportParameters(m_objReport, eleReport)

    Catch ex As Exception
      MsgBox(ex.Message)    'Catch com error
    End Try
    bDoneLoading = True   'loading report is complete

  End Sub

  Friend Function CheckForSubReport(
      ByRef p_objReport As ReportDocument) As Boolean

    Dim objSubReport As SubreportObject
    For Each objReportObject As ReportObject In p_objReport.ReportDefinition.ReportObjects
      If objReportObject.Kind = ReportObjectKind.SubreportObject Then
        objSubReport = CType(objReportObject, SubreportObject)
        m_strSubReportName = objSubReport.SubreportName
        Return True
      End If
    Next

    Return False

  End Function

  Friend Function LoadReportParameters(
      ByRef p_objReport As ReportDocument,
      ByVal p_eleReport As XmlNode) As Boolean

    Dim strParamName, strXmlParam, strInnerValue As String
    Dim crParameterFieldDefinition As ParameterFieldDefinition
    Dim crParameterFieldDefinitions As ParameterFieldDefinitions
    Dim crParameterValues As New ParameterValues
    Dim crParameterDiscreteValue As New ParameterDiscreteValue
    Dim eleReportParams As XmlNode

    If m_strReportTitle <> String.Empty Then p_objReport.SummaryInfo.ReportTitle = m_strReportTitle

    Try
      For nParamCount As Integer = 0 To p_objReport.DataDefinition.ParameterFields.Count - 1
        strParamName = p_objReport.DataDefinition.ParameterFields.Item(nParamCount).Name()     'get first pram
        crParameterFieldDefinitions = p_objReport.DataDefinition.ParameterFields     'define pram field definition
        crParameterFieldDefinition = crParameterFieldDefinitions.Item(strParamName)
        strXmlParam = strParamName.Replace("@", "")     'remove @ Symbol from pram name in crystal to match XML AES
        eleReportParams = p_eleReport.SelectSingleNode(strXmlParam)     'match crystal pram with XML pram AES
        If IsNothing(eleReportParams) Then
          ''Add for the print report summary AES
          If strXmlParam = "bPrintSummary" Then
            crParameterDiscreteValue.Value = CBool(m_bPRINT_REPORT_PARAMETERS)       'format string to Crystal Boolean
            crParameterValues = crParameterFieldDefinition.CurrentValues      'Set parameter value
            crParameterValues.Add(crParameterDiscreteValue)      'Add to Parm value
            crParameterFieldDefinition.ApplyCurrentValues(crParameterValues)      'Apply to report
          ElseIf Not p_objReport.DataDefinition.ParameterFields.Item(nParamCount).IsLinked() Then
            ' Erick Korsten ~ 09/02/04 ~ Only show error message if parameter is not linked. If the parameter is linked ...
            ' ... then it does not need to have a corresponding XML parameter. It could be linked to a database field (sub reports).
            MsgBox("Report Parameter " & strParamName & " ,does not match XML Parameter")     'There was no match
          End If
        Else
          'Error ID 1-5508 - Changed the eleReportParams to InnerText instead of InnerXml
          'to get rid of the amp; for the reports. MLG
          strInnerValue = eleReportParams.InnerText      'get XML pram inner node
          Select Case p_objReport.DataDefinition.ParameterFields.Item(nParamCount).ValueType()       'get pram value type for formatting inner node from a string value
            Case FieldValueType.CurrencyField       'Currency
              crParameterDiscreteValue.Value = CDbl(strInnerValue)        'format string to Crystal double
            Case FieldValueType.DateField, FieldValueType.DateTimeField       'Date
              crParameterDiscreteValue.Value = CDate(strInnerValue)        'format string to Crystal Date
            Case FieldValueType.NumberField       'Intenger
              crParameterDiscreteValue.Value = CInt(strInnerValue)        'format string to Crystal Intenger
            Case FieldValueType.StringField       'String
              crParameterDiscreteValue.Value = CStr(strInnerValue)        'format string to Crystal String
            Case FieldValueType.UnknownField       'Unknown
              crParameterDiscreteValue.Value = CObj(strInnerValue)        'format string to Crystal Unknown
            Case FieldValueType.BooleanField       'Boolean
              crParameterDiscreteValue.Value = CBool(strInnerValue)        'format string to Crystal Boolean
          End Select
          crParameterValues = crParameterFieldDefinition.CurrentValues      'Set parameter value
          crParameterValues.Add(crParameterDiscreteValue)      'Add to Parm value
          crParameterFieldDefinition.ApplyCurrentValues(crParameterValues)      'Apply to report
        End If
        Application.DoEvents()
      Next nParamCount
    Catch ex As Exception
      MsgBox(ex.Message)
      Return True
    End Try

    Return False

  End Function

End Class


Friend Class WaitThread
  '****************************************************************/
  '	WaitThread -  Used to show progress dialog
  '
  '	Called -	    from .net dll(RunReport)
  '****************************************************************/

  Friend bCancel As Boolean = False
  Friend bStop As Boolean = False


  Friend Enum enuMode
    ''' <summary>
    ''' This is used to identify that the value has not been set to a valid value yet
    ''' </summary>
    Invalid = -1
    MD_PROCESSREPORT    ' processing report
    MD_PROCESSDATA      ' processing data
    MD_LOADREPORT       ' loading report in to viewer
  End Enum

  Friend mode As enuMode

  Friend Sub WaitForm()

    Dim frmViewer As New frmReportViewer
    Dim frmProgress As New FRM_ReportProgress
    Dim nHeaderRecord As Integer = g_intHeaderRecordCount
    Dim nDetailRecord As Integer = g_intDetailRecordCount

    With frmProgress    ' load frmProgress and set values
      .ShowInTaskbar = True
      .Show()
      .BringToFront()
    End With

    Application.DoEvents()   'reset GUI

    ' Do While bCancel = False
    'frmProgress.BringToFront()
    'Select Case mode
    '  Case enuMode.MD_PROCESSREPORT     'when the report is being processed
    '    Application.DoEvents()
    '    If frmProgress.FormMode <> enuMode.MD_PROCESSREPORT Then frmProgress.FormMode = enuMode.MD_PROCESSREPORT

    '  Case enuMode.MD_PROCESSDATA     'After the data is retrieved and proccessing
    '    Application.DoEvents()
    '    If frmProgress.FormMode <> enuMode.MD_PROCESSDATA Then frmProgress.FormMode = enuMode.MD_PROCESSDATA
    '  Case enuMode.MD_LOADREPORT     'when the report and data is loading in the viewer
    '    Application.DoEvents()
    '    If frmProgress.FormMode <> enuMode.MD_LOADREPORT Then frmProgress.FormMode = enuMode.MD_LOADREPORT
    '    If nDetailRecord <= 0 Then
    '      If nHeaderRecord <= 0 Then
    '        nHeaderRecord = g_intHeaderRecordCount
    '      Else
    '        nHeaderRecord = nHeaderRecord - 1
    '        nDetailRecord = g_intDetailRecordCount
    '      End If
    '    Else
    '      nDetailRecord = nDetailRecord - 1       '
    '    End If

    '    frmProgress.HeaderNumber = nHeaderRecord     'add to text
    '    frmProgress.DetailNumber = nDetailRecord

    'End Select
    'If frmProgress.Cancel And bStop = False Then bStop = True

    'Application.DoEvents()
    ' Loop

  End Sub

End Class

'PGP(04/10/2006) - Interface for licence server events.
<GuidAttribute("0513EB77-45F7-44b0-890D-54C46FDF668C"),
InterfaceTypeAttribute(ComInterfaceType.InterfaceIsIDispatch)>
Friend Interface LicenseServerEvents
  Sub StateChanged(ByVal p_intServerState As Long)
End Interface

'****************************************************************/
'	LoadNETDLL - Com interface for Printing DLL
'
'	Called -	Call from client to create DLL
'****************************************************************/
'KPM.2005.03.30 A while back I commented this out and I should not have.
'This will install it in the registry and put the threading model to
'free, which allows simultaneous calls through two COM interfaces.
'<ComSourceInterfaces(GetType(LicenseServerEvents))> - connection to com event interface.
<Guid("54FEF6D5-85C4-4d72-A548-1B85B1ECF51E"),
ClassInterface(ClassInterfaceType.AutoDual)>
<ComSourceInterfaces(GetType(LicenseServerEvents))>
Friend Class LoadNETDLL

  Private m_strErrorXML As String
  Private m_hwPrintScreen As Long = 0
  Private m_strXMLLang As String

  'Private WithEvents m_objCallServer As New ShrNet32.CLS_CallServer
  'Public Delegate Sub StateChangedDelegate(ByVal p_intServerState As Long)

#Region " Dll Registration        "

#Region " Register        "

  ''' <summary>
  '''
  ''' </summary>
  ''' <param name="t"></param>
  ''' <remarks></remarks>
  <ComRegisterFunction()>
  Friend Shared Sub ComRegister(
      ByVal t As Type)

    Dim keyName As String = String.Format("CLSID\{0}\InprocServer32", t.GUID.ToString("B"))
    Using key As RegistryKey = Registry.ClassesRoot.OpenSubKey(keyName, True)
      With key
        .SetValue("CodeBase", Assembly.GetExecutingAssembly().CodeBase)
        .SetValue("ThreadingModel", "Free")
        .Close()
      End With
    End Using

  End Sub

#End Region

#Region " Unregister        "

  ''' <summary>
  '''
  ''' </summary>
  ''' <param name="t"></param>
  ''' <remarks></remarks>
  <ComUnregisterFunction()>
  Friend Shared Sub ComUnregister(
      ByVal t As Type)

    Dim keyName As String = String.Format("CLSID\{0}\InprocServer32", t.GUID.ToString("B"))
    Using key As RegistryKey = Registry.ClassesRoot.OpenSubKey(keyName, True)
      With key
        For Each s As String In .GetSubKeyNames()
          .DeleteSubKey(s, False)
        Next
        .Close()
      End With
    End Using

  End Sub

#End Region

#End Region

  Friend Function DoXMLCommandBackground(
      ByVal command As String,
      ByVal data As String,
      ByVal display As String) As String

    Dim frm As New frmCommandSend

    frm.Command = command
    frm.Data = data
    frm.lblStatus.Text = display
    frm.ShowDialog()

    Return frm.Return

  End Function

  Friend Function GetPrintScreenHandle() As Long

    Dim lReturn As Long = m_hwPrintScreen

    m_hwPrintScreen = 0

    Return lReturn

  End Function

  Friend Function LoadPrintBox(
      ByVal p_strXMLParameters As String,
      ByVal p_strCommand As String,
      ByVal p_strData As String,
      Optional ByVal p_strPrintPrams As String = "<ROOT></ROOT>",
      Optional ByVal p_bDeveloperViewer As Boolean = False) As Boolean
    '****************************************************************/
    '	LoadPrintBox - For printing reports, show print dialog form
    '
    '	Called -	Call from client side forms for printing reports
    '****************************************************************/

    Dim frmPrintBox As New NETDLL.frmPrintBox
    m_hwPrintScreen = 0
    frmPrintBox.ShowInTaskbar = False           ' don't show in taskbar
    frmPrintBox.DeveloperViewer = p_bDeveloperViewer   ' dev mode
    frmPrintBox.XMLParameters = p_strXMLParameters    ' report parms
    frmPrintBox.Command = p_strCommand          ' server command
    frmPrintBox.Data = p_strData             ' server data
    frmPrintBox.SetPrintBoxOptions(p_strPrintPrams)    ' set print box options
    g_bRunClientData = False   'use server data AES
    If frmPrintBox.ShowPrintDialog Then
      m_strErrorXML = frmPrintBox.DisplayDialog()    'show print dlg
      m_hwPrintScreen = frmPrintBox.ScreenHandle
      frmPrintBox.Close()
      frmPrintBox = Nothing
    Else
      m_strErrorXML = frmPrintBox.DisplayProcess()    'show print dlg
      m_hwPrintScreen = frmPrintBox.ScreenHandle
      frmPrintBox = Nothing
    End If

    System.GC.Collect()   'clean up

    Return True

  End Function

  Friend Function PrintReport(
      ByVal p_strXMLData As String,
      ByVal p_strXMLCRData As String,
      Optional ByVal strPrintPrams As String = "<ROOT></ROOT>",
      Optional ByVal bDeveloperViewer As Boolean = False) As Integer

    'CLS_Report.PreviewProgressedReport("", p_strXMLData, p_strXMLCRData)
    Return 0

    'Try
    '  Dim docReport As New XmlDocument

    '  Try
    '    docReport.LoadXml(p_strXMLData)
    '  Catch
    '    Return 4
    '  End Try

    '  Dim docCRData As New XmlDocument
    '  Try
    '    docCRData.LoadXml(p_strXMLData)
    '  Catch
    '    Return 5
    '  End Try

    '  Dim dat As New DataSet
    '  Try
    '    Dim stw As New StringReader(p_strXMLData)
    '    dat.ReadXml(stw)
    '  Catch
    '    Return 6
    '  End Try

    '  Dim docDialogInfo As New XmlDocument
    '  Try
    '    docDialogInfo.LoadXml(p_strXMLCRData)
    '  Catch
    '    Return 7
    '  End Try

    '  '*** Get custom print box title from XML
    '  Dim eleLang As XmlElement
    '  Dim strDlgTitle As String = String.Empty

    '  If Not IsNothing(docDialogInfo.DocumentElement("REPORT")) Then eleLang = docDialogInfo.DocumentElement("REPORT") ' get elements from XML

    '  If Not (IsNothing(eleLang)) Then
    '    If Not IsNothing(eleLang("REPORTTITLE")) Then
    '      strDlgTitle = eleLang("REPORTTITLE").InnerText
    '    Else
    '      If Not IsNothing(eleLang("FORMTITLE")) Then
    '        strDlgTitle = eleLang("FORMTITLE").InnerText
    '      End If
    '    End If
    '  End If
    '  '****************************************

    '  Dim pbx As New frmPrintBox
    '  g_bRunClientData = True
    '  pbx.DeveloperViewer = bDeveloperViewer         ' dev mode
    '  ' Erick Korsten - 07/19/04 - Pass the XML parameters to the print box class
    '  pbx.XMLParameters = p_strXMLCRData         ' report parms
    '  pbx.ReportTitle = strDlgTitle
    '  g_bVal = pbx.RunReportFromClientData(dat, p_strXMLCRData)
    '  If g_bVal Then
    '    pbx.SetPrintBoxOptions(strPrintPrams)          ' set print box options
    '    '*** set custom print box title.
    '    If Not (IsNothing(eleLang)) Then
    '      pbx.Text = strDlgTitle + " " + g_IDS_REPORTPRINTOPTIONS
    '      pbx.ReportTitle = strDlgTitle            ' EK - 07/19/04 - Set default report title to form title
    '    End If
    '    '*******************************
    '    ''1-7903 AES 10.28.03 need to check the edit bool to show print dialog
    '    If pbx.ShowPrintDialog Then
    '      m_strErrorXML = pbx.DisplayDialog()           ' Show print dlg
    '      pbx.Close()
    '    Else
    '      pbx.Cursor = Cursors.WaitCursor            ' Wait cursor
    '      ' m_strErrorXML = pbx.RunAll()  ' Dont show print dlg just print
    '      m_strErrorXML = pbx.DisplayProcess()    'DAJ 3/30/5 err: 1-15823 changed the function call here to make sure all the params needed for printing to files gets set
    '      pbx.Cursor = Cursors.Default             ' Wait cursor
    '    End If
    '  End If
    '  m_hwPrintScreen = pbx.ScreenHandle
    '  pbx = Nothing
    'Catch ex As Exception
    '  MessageBox.Show(ex.Message)
    'End Try

    'Return 0

  End Function

  Friend Function LoadDLL(
      ByVal p_strXMLLang As String,
      ByVal p_bolLoadLicenseServer As Boolean) As Boolean
    '****************************************************************/
    '	LoadDLL - Used to load Crystal and .net dll
    '
    '	Called -	Call from client side exe.
    '****************************************************************/
    'KPM.2005.03.29 Faster loading of crystal reports on startup

    m_strXMLLang = p_strXMLLang
    ThreadLoadDll()
    'KPM.2005.04.05 Removed for now to see if this is the problem.
    'Dim th As New System.Threading.Thread(AddressOf ThreadLoadDll)
    'th.Start()
    If p_bolLoadLicenseServer Then
      If m_objCallServer.LicenseServer Is Nothing OrElse Not m_objCallServer.LicenseServer.Running Then
        Try
          m_objCallServer.StartProcess()
        Catch ex As Exception
          MessageBox.Show(CLS_Math.CreateErrorMessage(ex), "Try Again later...")
          Return False
        End Try
      End If
    End If
    If Not m_strErrorXML Is Nothing AndAlso m_strErrorXML.Length > 0 Then Return False
    If p_bolLoadLicenseServer AndAlso (m_objCallServer Is Nothing OrElse Not m_objCallServer.Running) Then
      If Not m_objCallServer Is Nothing Then
        m_objCallServer.StopProcess()
      End If
      m_objCallServer = Nothing
      Return False
    End If
    Return True

  End Function

  Friend Sub ThreadLoadDll()

    Dim frmPrintBox As New frmPrintBox   'load print box
    frmPrintBox = Nothing
    Dim frmReportViewer As New NETDLL.frmReportViewer
    frmReportViewer = Nothing
    LoadLang(m_strXMLLang)

    Dim xmlstring As String = UTL_TransactedServer.DoXMLCommand("<LOADDOTNETCOMPONENTS>", "")   'send command to SQL server
    If Left(xmlstring, 7) = "!ERROR!" Then
      Dim szErrorMessage As String
      szErrorMessage = "Unable to initialize the connection to the "
      szErrorMessage += "SXMLPARS.DLL.  Please check your COM+ connection "
      szErrorMessage += "to SXMLPARS.DoXMLCommand or reinstall your server "
      szErrorMessage += "side components on your machine to make sure the "
      szErrorMessage += "DLL is installed correctly.  If you are still "
      szErrorMessage += "having trouble with this, Please call your system "
      szErrorMessage += "administrator to help you."
      MessageBox.Show(szErrorMessage, "Server Component Loading Error", MessageBoxButtons.OK, MessageBoxIcon.Error)
      m_strErrorXML = xmlstring
    End If
    System.GC.Collect()   'clean up

  End Sub

  Friend Function CheckError() As String
    ' ********************************************************
    ' * Procedure:    CheckError
    ' *
    ' * Description:  Checks to see if the server side returned any errors
    ' *
    ' * Parameters:   None
    ' *
    ' * Returns:      String - Blank=No Errors, Else Errors
    ' *
    ' * Date Created: 02/25/2003 by Peter Ringering
    ' ********************************************************

    'KPM, 10/17/2003, Sometimes this was nothing, which caused a fatal error on the
    'return doing the new method of connecting to COM dll's.
    Dim strErrorXml As String = m_strErrorXML

    m_strErrorXML = String.Empty

    If IsNothing(strErrorXml) Then
      Return String.Empty
    Else
      Return strErrorXml
    End If

  End Function

  Private Sub LoadLang(
      ByRef strXMLLang As String)
    '****************************************************************/
    '	PrintBox Class - Used load lang in gobals
    '
    '	Called -	    from .net dll()
    '****************************************************************/

    Dim DocXml As New XmlDocument        'xml dox
    Dim eleLang As XmlElement = Nothing
    'Dim eleLangPrams As XmlElement   'XML element

    Try
      DocXml.LoadXml(strXMLLang)          'set XMLstring in a doc
      eleLang = DocXml.DocumentElement("LANG")   'get elements from XML AES
    Catch ex As Exception

    End Try

    'set all gobal string for labels
    g_IDS_PRINTOPTIONDESC = GetXMLData(eleLang, "IDS_PRINTOPTIONDESC", "This is the description")
    g_IDS_PRINTER = GetXMLData(eleLang, "IDS_PRINTER", "Printer")
    g_IDS_SCREEN = GetXMLData(eleLang, "IDS_SCREEN", "Screen")
    g_IDS_FILE = GetXMLData(eleLang, "IDS_FILE", "File")
    g_IDS_LASERPRINTER = GetXMLData(eleLang, "IDS_LASERPRINTER", "Laser")
    g_IDS_DOTMATRIX = GetXMLData(eleLang, "IDS_DOTMATRIX", "Dot Matrix")
    g_IDS_NOOFCOPIES = GetXMLData(eleLang, "IDS_NOOFCOPIES", "Number of Copies")
    g_IDS_OK = GetXMLData(eleLang, "IDS_OK", "&Ok")
    g_IDS_CANCEL = GetXMLData(eleLang, "IDS_CANCEL", "Cancel")
    g_IDS_SETUPPRINTER = GetXMLData(eleLang, "IDS_SETUPPRINTER", "Setup Printers")
    g_IDS_CRYSTALVIEWER = GetXMLData(eleLang, "IDS_CRYSTALVIEWER", "Crystal Report Viewer")
    g_IDS_CURPRINTER = GetXMLData(eleLang, "IDS_CURPRINTER", "Current Printer")
    g_IDS_EXPORTFORMAT = GetXMLData(eleLang, "IDS_EXPORTFORMAT", "Export Format")
    g_IDS_WORDFORMAT = GetXMLData(eleLang, "IDS_WORDFORMAT", "Microsoft Word(*.doc)")
    g_IDS_PDFFORMAT = GetXMLData(eleLang, "IDS_PDFFORMAT", "Adobe Acrobat(*.pdf)")
    g_IDS_EXCELFORMAT = GetXMLData(eleLang, "IDS_EXCELFORMAT", "Microsoft Excel(*.xls)")
    g_IDS_HTML40FORMAT = GetXMLData(eleLang, "IDS_HTML40FORMAT", "HTML 4.0(*.html)")
    g_IDS_XMLFORMAT = GetXMLData(eleLang, "IDS_XMLFORMAT", "XML(*.xml)")
    g_IDS_EXPORT_FILENAME = GetXMLData(eleLang, "IDS_EXPORT_FILENAME", "Report")
    g_IDS_REPORTPRINTOPTIONS = GetXMLData(eleLang, "IDS_REPORTPRINTOPTIONS", "Print Options")
    g_IDS_PRINTERINVALID = GetXMLData(eleLang, "IDS_PRINTERINVALID", "Before you can perform printer-related tasks, you need to install a printer.")
    g_IDS_DEFAULT = GetXMLData(eleLang, "IDS_DEFAULT", "Default")
    'added for print report summary AES 6.16.04
    g_IDS_PRINT_REPORT_PARAMETERS = GetXMLData(eleLang, "IDS_GBL_PRINT_REPORT_PARAMS", "Print Report Summary")

  End Sub

  Protected Overrides Sub Finalize()

    If Not m_objCallServer Is Nothing AndAlso m_objCallServer.Running Then
      m_objCallServer.StopProcess()
    End If
    m_objCallServer = Nothing
    MyBase.Finalize()

  End Sub

  Friend Sub New()

  End Sub

  Private Function GetXMLData(
      ByVal p_eleLang As XmlElement,
      ByVal p_strItem As String,
      ByVal p_strDefault As String) As String

    If p_eleLang Is Nothing OrElse p_eleLang.SelectSingleNode(p_strItem) Is Nothing Then
      Return p_strDefault
    Else
      Return p_eleLang.SelectSingleNode(p_strItem).InnerXml
    End If

  End Function

  '#Region " License Server Code     "

  '#Region " Private Constants       "

  '  Private Const aparq As String = "8F4BBCC"
  '  Private Const apard As String = "b18639D"
  '  Private Const aparg As String = "EF6125B"
  '  Private Const aparh As String = "3047"
  '  Private Const apark As String = "BEE9149"

  '#End Region

  '  Public Event StateChanged As StateChangedDelegate

  '  Public Function GetLicense( _
  '      ByVal p_strMachineGUID As String, _
  '      ByVal p_strProcessGUID As String, _
  '      ByVal p_strModule As String, _
  '      ByVal p_strPlatform As String, _
  '      ByVal p_strProductLine As String, _
  '      ByVal p_intVersion As Integer) As String

  '    If m_objCallServer.LicenseServer Is Nothing OrElse Not m_objCallServer.LicenseServer.Running Then
  '      Try
  '        m_objCallServer.StartProcess()
  '      Catch ex As Exception
  '        MessageBox.Show(CLS_Math.CreateErrorMessage(ex), "Try Again later...")
  '        Return String.Empty
  '      End Try
  '    End If

  '    Dim obj As New CLS_Connect
  '    With obj

  '      Try
  '        .Machine = New Guid(p_strMachineGUID.Replace("_", "-"))
  '      Catch ex As Exception
  '        MessageBox.Show("Machine: " & ex.Message, "Licensing Error")
  '      End Try

  '      .SessionName = ShrNet32.CLS_CallServer.GetSessionName()

  '      Try
  '        .PID = New Guid(p_strProcessGUID.Replace("_", "-"))
  '      Catch ex As Exception
  '        MessageBox.Show("PID: " & ex.Message, "Licensing Error")
  '      End Try

  '      Try
  '        .Module = CType([Enum].Parse(.Module.GetType(), p_strModule, True), CMSUtil.CLS_LicenseServerConstructs.enuModule)
  '      Catch ex As Exception
  '        MessageBox.Show("Module: " & ex.Message, "Licensing Error")
  '      End Try

  '      Try
  '        .Platform = CType([Enum].Parse(.Platform.GetType(), p_strPlatform, True), CLS_LicenseServerConstructs.enuPlatforms)
  '      Catch ex As Exception
  '        MessageBox.Show("Platform: " & ex.Message, "Licensing Error")
  '      End Try

  '      Try
  '        .ProductType = CType([Enum].Parse(.ProductType.GetType(), p_strProductLine, True), CLS_LicenseServerConstructs.enuProductType)
  '      Catch ex As Exception
  '        .ProductType = CLS_LicenseServerConstructs.enuProductType.Invalid
  '      End Try

  '      .Version = p_intVersion
  '      .MachineName = SystemInformation.ComputerName

  '    End With

  '    m_objCallServer.Connect = obj
  '    Return m_objCallServer.SendToServer(obj.ToCommand(), True).OuterXml

  '  End Function

  '  Public Sub ReleaseLicense()

  '    If Not m_objCallServer Is Nothing AndAlso m_objCallServer.Running Then
  '      m_objCallServer.StopProcess()
  '    End If
  '    m_objCallServer = Nothing

  '  End Sub

  '  Public Function GetLicenses() As String

  '    If m_objCallServer.LicenseServer Is Nothing OrElse Not m_objCallServer.LicenseServer.Running Then
  '      Try
  '        m_objCallServer.StartProcess()
  '      Catch ex As Exception
  '        MessageBox.Show(CLS_Math.CreateErrorMessage(ex), "Try Again later...")
  '        Return String.Empty
  '      End Try
  '    End If

  '    If Not m_objCallServer.LicenseServer.Connection.Connected Then
  '      Return "<ROOT><ERROR>License Server is not running at this time try again later.</ERROR></ROOT>"
  '    End If

  '    Dim obj As New CLS_License(aparg & apark & apard & aparq & aparh)

  '    Return m_objCallServer.SendToServer(obj.ToCommand(), True).OuterXml

  '  End Function

  '  Public Function GetCustomer() As String

  '    If m_objCallServer.LicenseServer Is Nothing OrElse Not m_objCallServer.LicenseServer.Running Then
  '      Try
  '        m_objCallServer.StartProcess()
  '      Catch ex As Exception
  '        MessageBox.Show(CLS_Math.CreateErrorMessage(ex), "Try Again later...")
  '        Return String.Empty
  '      End Try
  '    End If

  '    If Not m_objCallServer.LicenseServer.Connection.Connected Then
  '      Return "<ROOT><ERROR>License Server is not running at this time try again later.</ERROR></ROOT>"
  '    End If

  '    Dim obj As New CLS_Customer
  '    obj.CommandType = CLS_BaseCommand.enuCommandType.Server

  '    Return m_objCallServer.SendToServer(obj.ToCommand(), True).OuterXml

  '  End Function

  '  Private Sub m_objCallServer_DataReturned( _
  '      ByVal p_strData As String) _
  '      Handles m_objCallServer.DataReturned

  '  End Sub

  '  Private Sub m_objCallServer_StateChanged( _
  '      ByVal p_enmServerState As ShrNet32.CLS_CallServer.ServerState) _
  '      Handles m_objCallServer.StateChanged

  '    RaiseEvent StateChanged(CLng(p_enmServerState))

  '  End Sub

  '#End Region

End Class
