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

Imports ShrNet32
Imports System.Xml

#End Region

#Region " Main Code ============================================================== "

''' <summary>
''' Wizard Page for License Key Entry
''' </summary>
''' <seealso cref="CTL_WizardPageBase" />
Public Class WPAC_CDKeyEntry

  Inherits CTL_WizardPageBase

#Region " Windows Form Designer generated code =================================== "

  ''' <summary>
  ''' Initializes a new instance of the <see cref="WPAC_CDKeyEntry"/> class.
  ''' </summary>
  ''' <param name="p_objCallServer">The p object call server.</param>
  ''' <param name="p_frmWizardBase">The p FRM wizard base.</param>
  Public Sub New(
      ByRef p_objCallServer As CLS_CallServer,
      ByRef p_frmWizardBase As FRM_WizardBase)

    MyBase.New(p_frmWizardBase)

    'This call is required by the Windows Form Designer.
    InitializeComponent()

    'Add any initialization after the InitializeComponent() call
    m_objCallServer = p_objCallServer

  End Sub

  'Form overrides dispose to clean up the component list.
  ''' <summary>
  ''' Releases the unmanaged resources used by the <see cref="T:System.Windows.Forms.Control" /> and its child controls and optionally releases the managed resources.
  ''' </summary>
  ''' <param name="disposing"><see langword="true" /> to release both managed and unmanaged resources; <see langword="false" /> to release only unmanaged resources.</param>
  Protected Overloads Overrides Sub Dispose(
      ByVal disposing As Boolean)

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
  Friend WithEvents lblStep1 As Label
  Friend WithEvents lblStep2 As Label
  Friend WithEvents CtL_3DLine2 As CTL_3DLine
  Friend WithEvents cmdAdd As Button
  Friend WithEvents ctlPackages As CTL_PackagesInstalledGrid
  Friend WithEvents CtL_CDKeyValidator1 As CTL_CDKeyValidator

  ''' <summary>
  ''' Initializes the component.
  ''' </summary>
  <DebuggerStepThrough()>
  Private Sub InitializeComponent()

    lblStep1 = New Label
    lblStep2 = New Label
    CtL_3DLine2 = New CTL_3DLine
    cmdAdd = New Button
    ctlPackages = New CTL_PackagesInstalledGrid
    CtL_CDKeyValidator1 = New CTL_CDKeyValidator
    SuspendLayout()
    '
    'lblStep1
    '
    lblStep1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) Or
      System.Windows.Forms.AnchorStyles.Right), AnchorStyles)
    lblStep1.Location = New Point(5, 6)
    lblStep1.Name = "lblStep1"
    lblStep1.Size = New Size(624, 16)
    lblStep1.TabIndex = 9
    lblStep1.Text = "Enter your product keys and user keys in the following boxes and select the Unlock" &
    " button to add it to the table below."
    '
    'lblStep2
    '
    lblStep2.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                Or System.Windows.Forms.AnchorStyles.Right), AnchorStyles)
    lblStep2.Location = New Point(5, 160)
    lblStep2.Name = "lblStep2"
    lblStep2.Size = New Size(624, 16)
    lblStep2.TabIndex = 14
    lblStep2.Text = "Unlocked Products"
    '
    'CtL_3DLine2
    '
    CtL_3DLine2.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                Or System.Windows.Forms.AnchorStyles.Right), AnchorStyles)
    CtL_3DLine2.DarkColor = System.Drawing.SystemColors.ControlDark
    CtL_3DLine2.LightColor = System.Drawing.SystemColors.ControlLight
    CtL_3DLine2.Location = New Point(-3, 152)
    CtL_3DLine2.Name = "CtL_3DLine2"
    CtL_3DLine2.Size = New Size(638, 2)
    CtL_3DLine2.TabIndex = 13
    CtL_3DLine2.TabStop = False
    '
    'cmdAdd
    '
    cmdAdd.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), AnchorStyles)
    cmdAdd.Enabled = False
    cmdAdd.FlatStyle = System.Windows.Forms.FlatStyle.System
    cmdAdd.Location = New Point(457, 28)
    cmdAdd.Name = "cmdAdd"
    cmdAdd.Size = New Size(168, 24)
    cmdAdd.TabIndex = 12
    cmdAdd.Text = "&Unlock"
    '
    'ctlPackages
    '
    ctlPackages.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                Or System.Windows.Forms.AnchorStyles.Left) _
                Or System.Windows.Forms.AnchorStyles.Right), AnchorStyles)
    ctlPackages.AutoScroll = True
    ctlPackages.Location = New Point(15, 180)
    ctlPackages.Name = "ctlPackages"
    ctlPackages.Size = New Size(612, 144)
    ctlPackages.TabIndex = 15
    '
    'CtL_CDKeyValidator1
    '
    CtL_CDKeyValidator1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                Or System.Windows.Forms.AnchorStyles.Right), AnchorStyles)
    CtL_CDKeyValidator1.AutoScroll = True
    CtL_CDKeyValidator1.AutoScrollMinSize = New Size(424, 108)
    CtL_CDKeyValidator1.Location = New Point(33, 28)
    CtL_CDKeyValidator1.Name = "CtL_CDKeyValidator1"
    CtL_CDKeyValidator1.ReadOnly = False
    CtL_CDKeyValidator1.Size = New Size(596, 120)
    CtL_CDKeyValidator1.TabIndex = 11
    CtL_CDKeyValidator1.Text = "----"
    '
    'WPAC_CDKeyEntry
    '
    Controls.Add(lblStep1)
    Controls.Add(lblStep2)
    Controls.Add(CtL_3DLine2)
    Controls.Add(cmdAdd)
    Controls.Add(ctlPackages)
    Controls.Add(CtL_CDKeyValidator1)
    Name = "WPAC_CDKeyEntry"
    Size = New Size(632, 332)
    ResumeLayout(False)

  End Sub

#End Region

#Region " Class Level Variables ================================================== "

  ''' <summary>
  ''' The progress form
  ''' </summary>
  Private m_frmProgress As New FRM_TopMostProgress

  ''' <summary>
  ''' The call server object
  ''' </summary>
  Private WithEvents m_objCallServer As CLS_CallServer

  ''' <summary>
  ''' The continue progress
  ''' </summary>
  Private m_bolContinueProgress As Boolean = True

  ''' <summary>
  ''' The license object
  ''' </summary>
  Private m_objLicenses As CLS_License

#End Region

#Region " Private Constants ====================================================== "

  Private Const m_strLoc As String = "WPAC_CDKeyEntry"  ' Registry Entry Key

  Private Const aparq As String = "8F4BBCC"
  Private Const apard As String = "b18639D"
  Private Const aparg As String = "EF6125B"
  Private Const aparh As String = "3047"
  Private Const apark As String = "BEE9149"

#End Region

#Region " Class Routines ========================================================= "

  ''' <summary>
  ''' Reloads the ListView.
  ''' </summary>
  Private Sub ReloadListView()

    ' Initialize form values to blank
    ctlPackages.Clear()

    Dim stuRegEntries() As CLS_ServerInstallation

    m_objLicenses = New CLS_License(aparg & apark & apard & aparq & aparh, 4096, enuPlatforms.Invalid)
    With m_objLicenses
      .TenantID = If(UseTenants, TenantID, String.Empty)
      ' Call the license server and get a list of all of the CD Keys registered with it.
      Dim xmlData As XmlDocument = m_objCallServer.SendToServer(.ToCommand)
      .LoadXML(xmlData)

      ' Retrieve and Add 64 bit entries to list
      stuRegEntries = .ServerInstallation(enuPlatforms.CMS_64Bit)
      LoadListView(stuRegEntries)

      ' Retrieve and Add 32 bit entries to list
      stuRegEntries = .ServerInstallation(enuPlatforms.CMS_32Bit)
      LoadListView(stuRegEntries)

      ' Retrieve and Add 16 bit entries to list
      stuRegEntries = .ServerInstallation(enuPlatforms.CMS_16Bit)
      LoadListView(stuRegEntries)
    End With

  End Sub

  ''' <summary>
  ''' Loads the ListView.
  ''' </summary>
  ''' <param name="stuRegEntries">The reg entries.</param>
  Private Sub LoadListView(
      ByRef stuRegEntries() As CLS_ServerInstallation)

    ctlPackages.AddRange(stuRegEntries)
    m_frmWizardBase.cmdNext.Enabled = ctlPackages.Activate

  End Sub

  ''' <summary>
  ''' Populates the cd key information.
  ''' </summary>
  ''' <param name="sender">The sender.</param>
  ''' <param name="e">The <see cref="CDKeyTextEventArgs"/> instance containing the event data.</param>
  Private Sub PopulateCDKeyInfo(
      ByVal sender As Object,
      ByVal e As CDKeyTextEventArgs) Handles CtL_CDKeyValidator1.CMSCDKeyEntered

    Dim objKey As New CLS_CDKey(e.Text)

    'Is this a valid CD Key?
    If Not objKey.InvalidKey Then
      cmdAdd.Enabled = True
    ElseIf Not CtL_CDKeyValidator1.IgnoreFullField AndAlso CtL_CDKeyValidator1.Text.Replace("-", "").Replace(" ", "").Trim().Length = 25 Then
      'NO
      MsgBox("Invalid CD Key please re-enter!")
      m_frmWizardBase.cmdSpecial.Focus()
      CtL_CDKeyValidator1.Focus()
    End If

  End Sub

  ''' <summary>
  ''' Handles the CMSCDKeyValid event of the CtL_CDKeyValidator1 control.
  ''' </summary>
  ''' <param name="sender">The source of the event.</param>
  ''' <param name="e">The <see cref="CDKeyEventArgs"/> instance containing the event data.</param>
  Private Sub CtL_CDKeyValidator1_CMSCDKeyValid(
      ByVal sender As Object,
      ByVal e As CDKeyEventArgs) _
      Handles CtL_CDKeyValidator1.CMSCDKeyValid

    cmdAdd.Enabled = e.Valid

  End Sub

  ''' <summary>
  ''' Handles the Click event of the cmdAdd control.
  ''' </summary>
  ''' <param name="sender">The source of the event.</param>
  ''' <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
  Private Sub cmdAdd_Click(
      ByVal sender As Object,
      ByVal e As EventArgs) Handles cmdAdd.Click

    CtL_CDKeyValidator1.Enabled = False
    Dim strCDKey As String = CtL_CDKeyValidator1.Text
    Dim objKey As New CLS_CDKey(strCDKey)

    'Add Entry to Registry!
    Dim obj As New CLS_Register(UseTenants, TenantID, objKey, New CLS_ActivationKey)
    Dim xmlData As XmlDocument = m_objCallServer.SendToServer(obj.ToCommand)
    If Exists(xmlData, "valid") Then
      CtL_CDKeyValidator1.Clear()
    ElseIf Exists(xmlData, "error") Then
      Dim strMessage As String = "License Server Error.  " & vbCrLf &
                                 "  Error: " & Xml.Text(xmlData, "error") & vbCrLf &
                                 "  Message: " & Xml.Text(xmlData, "message") & vbCrLf &
                                 "  Source: " & Xml.Text(xmlData, "source") & vbCrLf &
                                 "  Stack Trace: " & Xml.Text(xmlData, "stacktrace") &
                                 If(Exists(xmlData, "targetsite"), vbCrLf & "  Target Site: " & Xml.Text(xmlData, "targetsite"), "")
      MsgBox(strMessage, MsgBoxStyle.Critical, "License Server Error")
      m_frmWizardBase.cmdSpecial.Focus()
      CtL_CDKeyValidator1.Enabled = True
      CtL_CDKeyValidator1.Focus()
      Exit Sub
    Else
      MsgBox("Error during installation!")
      m_frmWizardBase.cmdSpecial.Focus()
      CtL_CDKeyValidator1.Enabled = True
      CtL_CDKeyValidator1.Focus()
      Exit Sub
    End If

    'Reload Table
    ReloadListView()

    'Reset Buttons and Displays
    cmdAdd.Enabled = False
    CtL_CDKeyValidator1.Enabled = True
    CtL_CDKeyValidator1.Clear()
    CtL_CDKeyValidator1.Focus()

  End Sub

  ''' <summary>
  ''' Handles the Click event of the cmdActivateSelected control.
  ''' </summary>
  ''' <param name="sender">The source of the event.</param>
  ''' <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
  Private Sub cmdActivateSelected_Click(
      ByVal sender As Object,
      ByVal e As EventArgs)

    Dim frmActivate As New WMAC_ActivationWizard(m_objCallServer, WMAC_ActivationWizard.enuWizardType.KeyActivation)

    frmActivate.ShowDialog(Me)

    ReloadListView()

  End Sub

#End Region

#Region " Overrides ============================================================== "

  'Use this Override to setup anything that is special for
  'this wizard page
  ''' <summary>
  ''' Setup the wizard page.
  ''' </summary>
  Public Overrides Sub SetupWizardPage()

    With m_frmWizardBase
      m_NextText = "Activate"
      .cmdNext.Enabled = False
    End With

    CtL_CDKeyValidator1.Clear()

    'Reload List view with Registry entries
    ReloadListView()

  End Sub

  ''' <summary>
  ''' Specials the button clicked.
  ''' </summary>
  ''' <param name="sender">The sender.</param>
  ''' <param name="e">The <see cref="T:System.EventArgs" /> instance containing the event data.</param>
  Public Overrides Sub SpecialButtonClicked(
      ByVal sender As Object,
      ByVal e As EventArgs)

    Try
      If m_frmWizardBase.cmdNext.Enabled AndAlso
         m_frmWizardBase.cmdNext.Visible Then
        If MessageBox.Show("You have products which have not been activated, would you like to activate them now?", "Do you want to activate your CMS Products?", MessageBoxButtons.YesNo) = MsgBoxResult.Yes Then
          cmdActivateSelected_Click(sender, e)
          Exit Sub
        End If
      ElseIf ctlPackages.lsvPackages.Items.Count = 0 OrElse
             CtL_CDKeyValidator1.IsCDKeyValid Then
        MsgBox("Select the unlock button to activate the license key(s) included with your product shipment. The license key(s) is printed on the CD package. Enter the key and click the Unlock button until you have activated all keys; then, click OK to continue.", MsgBoxStyle.Exclamation, "License Key Was Not Activated")
        If cmdAdd.CanFocus Then
          cmdAdd.Focus()
        ElseIf CtL_CDKeyValidator1.CanFocus Then
          CtL_CDKeyValidator1.Clear()
          CtL_CDKeyValidator1.Focus()
        End If

        Exit Sub
      End If
    Catch ex As Exception
      m_bolContinueProgress = False
      Application.DoEvents()
      MsgBox("Failed to configure client components!" & vbCrLf & ex.Message)
    End Try

    m_frmWizardBase.Close()

  End Sub

#End Region

End Class

#End Region
