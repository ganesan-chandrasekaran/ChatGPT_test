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

Imports CMSUtil.Commands.CLS_Modules
Imports CMSUtil.CLS_LicenseServerConstructs
Imports CMSUtil.CLS_Math
Imports System.IO

#End Region

#Region " Main Code ============================================================== "

Namespace Keys

  ''' <summary>
  ''' Class for processing a License Key
  ''' </summary>
  Public Class CLS_CDKey

#Region " Private  Constants ===================================================== "

    Private Const parq As String = "044AA2A"
    Private Const pard As String = "eaAB8FA"
    Private Const parg As String = "822FAB9"
    Private Const parh As String = "898D"
    Private Const park As String = "D715D45"

    Private Const ValidCharacters As String = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ"

    Private Const LenCRC As Integer = 12
    Private Const LenSerial As Integer = 24
    Private Const LenCustom As Integer = 24
    Private Const LenUsers As Integer = 32
    Private Const LenModules As Integer = 32
    Private Const LenTimeLimit As Integer = 12
    Private Const LenVersion As Integer = 12

#End Region

#Region " Private Structures ===================================================== "

    ''' <summary>
    ''' Structure to hold the parts of the key
    ''' </summary>
    Private Structure stuCDKeyParts
      ''' <summary>
      ''' The license key
      ''' </summary>
      Dim CDKey As String
      ''' <summary>
      ''' The key type
      ''' </summary>
      Dim KeyType As enuKeyType
      ''' <summary>
      ''' The key base
      ''' </summary>
      Dim KeyBase As enuKeyType
      ''' <summary>
      ''' The is the key automatic activated?
      ''' </summary>
      Dim AutoActivated As Boolean
      ''' <summary>
      ''' The platform
      ''' </summary>
      Dim Platform As enuPlatforms
      ''' <summary>
      ''' The version
      ''' </summary>
      ''' <remarks>
      ''' Can't be larger than 4095
      ''' </remarks>
      Dim Version As Integer  ' Can't be larger than 4095
      ''' <summary>
      ''' The users
      ''' </summary>
      Dim Users As Long
      ''' <summary>
      ''' The product type (Act or Fund)
      ''' </summary>
      Dim ProductType As enuProductType
      ''' <summary>
      ''' The modules
      ''' </summary>
      Dim Modules As String
      ''' <summary>
      ''' The not for resale
      ''' </summary>
      Dim NotForResale As Boolean
      ''' <summary>
      ''' The is a business partner order?
      ''' </summary>
      Dim BPOrder As Boolean
      ''' <summary>
      ''' The source
      ''' </summary>
      Dim Source As Boolean
      ''' <summary>
      ''' The upgrade
      ''' </summary>
      Dim Upgrade As Boolean
      ''' <summary>
      ''' The demo
      ''' </summary>
      Dim Demo As Boolean
      ''' <summary>
      ''' The fund
      ''' </summary>
      Dim Fund As Boolean
      ''' <summary>
      ''' The serial number
      ''' </summary>
      Dim SerialNumber As Long
      ''' <summary>
      ''' The incrementer
      ''' </summary>
      Dim Incrementer As Long
      ''' <summary>
      ''' The terminal identifier
      ''' </summary>
      Dim TerminalID As Long
      ''' <summary>
      ''' The maximum terminal
      ''' </summary>
      Dim MaxTerminal As Long
      ''' <summary>
      ''' The custom mod
      ''' </summary>
      Dim CustomMod As Long
      ''' <summary>
      ''' The time limit
      ''' </summary>
      Dim TimeLimit As Long
      ''' <summary>
      ''' The invalid key
      ''' </summary>
      Dim InvalidKey As Boolean
      ''' <summary>
      ''' The CRC
      ''' </summary>
      Dim CRC As Long
      ''' <summary>
      ''' The parity
      ''' </summary>
      Dim Parity As Boolean
      ''' <summary>
      ''' The parsing
      ''' </summary>
      Dim Parsing As Boolean
      ''' <summary>
      ''' The time extension
      ''' </summary>
      Dim TimeExtension As Boolean
    End Structure

#End Region

#Region " Private Variables ====================================================== "

    Private m_stuKey As stuCDKeyParts
    Private m_bolDirty As Boolean
    Private m_strKeyArray(124) As Char
    Private m_strError As String

#End Region

#Region " Constructor ============================================================ "

    ''' <summary>
    ''' Initializes a new instance of the <see cref="CLS_CDKey"/> class.
    ''' </summary>
    ''' <param name="p_strKey">The key.</param>
    ''' <param name="p_intMaxTerminal">The maximum terminal.</param>
    ''' <param name="p_intTerminalID">The terminal identifier.</param>
    Public Sub New(
        Optional ByVal p_strKey As String = "",
        Optional ByVal p_intMaxTerminal As Integer = 1,
        Optional ByVal p_intTerminalID As Integer = 0)

      Randomize(Timer)

      m_stuKey.MaxTerminal = If(p_intMaxTerminal < 1, 1, p_intMaxTerminal)
      m_stuKey.TerminalID = p_intTerminalID
      m_stuKey.Incrementer = 1

      m_stuKey.Modules = SetModuleFlag(enuModule.Controller, MakeZeroFlags())
      If Not String.IsNullOrEmpty(p_strKey) Then
        Key = p_strKey
      End If

    End Sub

#End Region

#Region " Public Properties ====================================================== "

    ''' <summary>
    ''' Gets or sets the key.
    ''' </summary>
    ''' <value>
    ''' The key.
    ''' </value>
    Public Property Key() As String

      Get

        If m_bolDirty Then
          m_stuKey.CDKey = GenerateKey()
          m_bolDirty = False
        End If

        If m_stuKey.CDKey.Length = 25 Then m_stuKey.CDKey = AddHyphenToKey(m_stuKey.CDKey)

        Return m_stuKey.CDKey

      End Get
      Set(ByVal Value As String)

        If m_stuKey.CDKey <> Value AndAlso Value <> "----" AndAlso Value.Replace("-", "").Length = 25 Then
          m_bolDirty = False
          m_stuKey.CDKey = Value
          ParseKey()
          If InvalidKey Then
            m_stuKey.CDKey = Nothing
          End If
          m_bolDirty = False
        ElseIf Value.Length > 0 AndAlso m_stuKey.CDKey <> Value Then
          m_stuKey.InvalidKey = True
        End If

        If m_stuKey.CDKey Is Nothing Then
          m_stuKey.CDKey = "xxxxx-xxxxx-xxxxx-xxxxx-xxxxx"
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the type of the key.
    ''' </summary>
    ''' <value>
    ''' The type of the key.
    ''' </value>
    Public Property KeyType() As enuKeyType

      Get

        Return m_stuKey.KeyType

      End Get
      Set(ByVal Value As enuKeyType)

        If m_stuKey.KeyType <> Value Then
          m_stuKey.KeyType = Value
          m_stuKey.KeyBase = CType(((Value - 1) Mod 3) + 1, enuKeyType)
          m_stuKey.AutoActivated = (Value > enuKeyType.Custom)
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the type of the key base.
    ''' </summary>
    ''' <value>
    ''' The key base.
    ''' </value>
    ''' <remarks>
    ''' This returns the base type of the key, so if it is a Product or AutoProduct type, it
    ''' returns Product.
    ''' </remarks>
    Public Property KeyBase() As enuKeyType

      Get

        Return m_stuKey.KeyBase

      End Get
      Set(ByVal Value As enuKeyType)

        Dim intVal As Integer = ((Value - 1) Mod 3) + 1
        If m_stuKey.KeyBase <> intVal Then
          m_stuKey.KeyBase = CType(intVal, enuKeyType)
          KeyType = CType(KeyBase + (3 * If(Auto, 1, 0)), enuKeyType)
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether this <see cref="CLS_CDKey"/> is automatically activated.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if automatic; otherwise, <c>false</c>.
    ''' </value>
    Public Property [Auto]() As Boolean

      Get

        Return m_stuKey.AutoActivated

      End Get
      Set(ByVal Value As Boolean)

        If m_stuKey.AutoActivated <> Value Then
          m_stuKey.AutoActivated = Value
          KeyType = CType(KeyBase + (3 * If(m_stuKey.AutoActivated, 1, 0)), enuKeyType)
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the platform.
    ''' </summary>
    ''' <value>
    ''' The platform.
    ''' </value>
    Public Property Platform() As enuPlatforms

      Get

        Return m_stuKey.Platform

      End Get
      Set(ByVal Value As enuPlatforms)

        If m_stuKey.Platform <> Value Then
          m_stuKey.Platform = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets the platform description.
    ''' </summary>
    ''' <value>
    ''' The platform description.
    ''' </value>
    Public ReadOnly Property PlatformDescription() As String

      Get

        Return GetPlatformDescription(Platform)

      End Get

    End Property

    ''' <summary>
    ''' Gets or sets the version.
    ''' </summary>
    ''' <value>
    ''' The version.
    ''' </value>
    ''' <exception cref="Exception">
    ''' Version number has to be at least 1.
    ''' or
    ''' Maximum version number you can set is " &amp; ((2 ^ LenVersion) - 1).ToString &amp; ".
    ''' </exception>
    Public Property Version() As Integer

      Get

        Return m_stuKey.Version

      End Get
      Set(ByVal Value As Integer)

        If KeyBase <> enuKeyType.Users Then
          If Value < 1 Then Throw New Exception("Version number has to be at least 1.")
          If Value > (2 ^ LenVersion) - 1 Then Throw New Exception($"Maximum version number you can set is {(2 ^ LenVersion) - 1}.")
        End If

        If m_stuKey.Version <> Value Then
          m_stuKey.Version = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets the version number.
    ''' </summary>
    ''' <value>
    ''' The version number.
    ''' </value>
    Public ReadOnly Property VersionNumber() As String

      Get

        Return GetVersionNumber(Version)

      End Get

    End Property

    ''' <summary>
    ''' Gets or sets the users.
    ''' </summary>
    ''' <value>
    ''' The users.
    ''' </value>
    ''' <exception cref="Exception">
    ''' You have to have at least one user selected for this key.
    ''' or
    ''' More users selected than can be set for a system. A maximum of " &amp; ((2 ^ LenUsers) - 1).ToString &amp; " users can be selected.
    ''' </exception>
    Public Property Users() As Long

      Get

        Return If(KeyBase = enuKeyType.Product, 1, If(KeyBase = enuKeyType.Custom, -1, m_stuKey.Users))

      End Get
      Set(ByVal Value As Long)

        If KeyBase = enuKeyType.Users Then
          If Value < 1 Then Throw New Exception("You have to have at least one user selected for this key.")
          If Value > (2 ^ LenUsers) - 1 Then Throw New Exception($"More users selected than can be set for a system. A maximum of {(2 ^ LenUsers) - 1} users can be selected.")
        End If

        If m_stuKey.Users.CompareTo(Value) <> 0 Then
          m_stuKey.Users = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets the user description.
    ''' </summary>
    ''' <value>
    ''' The user description.
    ''' </value>
    Public ReadOnly Property UserDescription() As String

      Get

        Return GetUserDescription(Users)

      End Get

    End Property

    ''' <summary>
    ''' Gets the key description.
    ''' </summary>
    ''' <value>
    ''' The key description.
    ''' </value>
    Public ReadOnly Property KeyDescription() As String

      Get

        Select Case KeyBase
          Case enuKeyType.Product, enuKeyType.Custom
            Dim strModules As String = String.Empty

            For Each enmModule As enuModule In Modules
              strModules &= $"{GetModuleDescription(enmModule)}, "
            Next

            If strModules Is Nothing Then
              Return String.Empty
            Else
              Return strModules.Substring(0, strModules.Length - 2)
            End If
          Case enuKeyType.Users
            ' TODO: Add Code here for proper return
            Return String.Empty
          Case Else
            Return String.Empty
        End Select

      End Get

    End Property

    ''' <summary>
    ''' Gets or sets the type of the product.
    ''' </summary>
    ''' <value>
    ''' The type of the product.
    ''' </value>
    Public Property ProductType() As enuProductType

      Get

        Return m_stuKey.ProductType

      End Get
      Set(ByVal Value As enuProductType)

        If m_stuKey.ProductType <> Value Then
          m_stuKey.ProductType = Value
          Select Case m_stuKey.ProductType
            Case enuProductType.Act
              Fund = False
            Case enuProductType.Fund
              Fund = True
          End Select
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the modules.
    ''' </summary>
    ''' <value>
    ''' The modules.
    ''' </value>
    Public Property Modules() As enuModule()

      Get

        Return GetModuleFlags(m_stuKey.Modules)

      End Get
      Set(ByVal Value As enuModule())

        Dim strModules As String = GetModuleFlags(Value)
        If m_stuKey.Modules <> strModules Then
          m_stuKey.Modules = SetModuleFlag(enuModule.Controller, strModules)
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether not for resale.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if not for resale; otherwise, <c>false</c>.
    ''' </value>
    Public Property NotForResale() As Boolean

      Get

        Return m_stuKey.NotForResale

      End Get
      Set(ByVal Value As Boolean)

        If m_stuKey.NotForResale <> Value Then
          m_stuKey.NotForResale = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether bp order.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if bp order; otherwise, <c>false</c>.
    ''' </value>
    Public Property BPOrder() As Boolean

      Get

        Return m_stuKey.BPOrder

      End Get
      Set(ByVal Value As Boolean)

        If m_stuKey.BPOrder <> Value Then
          m_stuKey.BPOrder = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether this <see cref="CLS_CDKey"/> is source.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if source; otherwise, <c>false</c>.
    ''' </value>
    Public Property Source() As Boolean

      Get

        Return m_stuKey.Source

      End Get
      Set(ByVal Value As Boolean)

        m_stuKey.Source = Value

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether this <see cref="CLS_CDKey"/> is upgrade.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if upgrade; otherwise, <c>false</c>.
    ''' </value>
    Public Property Upgrade() As Boolean

      Get

        Return m_stuKey.Upgrade

      End Get
      Set(ByVal Value As Boolean)

        If m_stuKey.Upgrade <> Value Then
          m_stuKey.Upgrade = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether this <see cref="CLS_CDKey"/> is demo.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if demo; otherwise, <c>false</c>.
    ''' </value>
    Public Property Demo() As Boolean

      Get

        Return m_stuKey.Demo

      End Get
      Set(ByVal Value As Boolean)

        If m_stuKey.Demo <> Value Then
          m_stuKey.Demo = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether this <see cref="CLS_CDKey"/> is fund.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if fund; otherwise, <c>false</c>.
    ''' </value>
    Public Property Fund() As Boolean

      Get

        Return m_stuKey.Fund

      End Get
      Set(ByVal Value As Boolean)

        If m_stuKey.Fund <> Value Then
          m_stuKey.Fund = Value
          ProductType = If(Fund, enuProductType.Fund, enuProductType.Act)
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the serial number.
    ''' </summary>
    ''' <value>
    ''' The serial number.
    ''' </value>
    Public Property SerialNumber() As Long

      Get

        CalcSerial()
        Return m_stuKey.SerialNumber

      End Get
      Set(ByVal Value As Long)

        If m_stuKey.SerialNumber.CompareTo(Value) <> 0 Then
          m_stuKey.SerialNumber = Value
          ParseSerial()
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the incrementer.
    ''' </summary>
    ''' <value>
    ''' The incrementer.
    ''' </value>
    Public Property Incrementer() As Long

      Get

        If m_stuKey.MaxTerminal = 1 Then
          Return CLng(m_stuKey.Incrementer Mod 2 ^ LenSerial)
        Else
          Return CLng(m_stuKey.Incrementer Mod (LenSerial - (Math.Floor(Math.Log(m_stuKey.MaxTerminal, 2)) + 1)))
        End If

      End Get
      Set(ByVal Value As Long)

        If m_stuKey.Incrementer.CompareTo(Value) <> 0 Then
          m_stuKey.Incrementer = Value
          CalcSerial()
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the terminal identifier.
    ''' </summary>
    ''' <value>
    ''' The terminal identifier.
    ''' </value>
    Public Property TerminalID() As Long

      Get

        Return m_stuKey.TerminalID

      End Get
      Set(ByVal Value As Long)

        If m_stuKey.TerminalID <> Value Then
          m_stuKey.TerminalID = Value
          CalcSerial()
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the maximum terminals.
    ''' </summary>
    ''' <value>
    ''' The maximum terminals.
    ''' </value>
    Public Property MaxTerminals() As Long

      Get

        Return m_stuKey.MaxTerminal

      End Get
      Set(ByVal Value As Long)

        If m_stuKey.MaxTerminal <> Value Then
          m_stuKey.MaxTerminal = Value
          CalcSerial()
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Calculates the serial.
    ''' </summary>
    Private Sub CalcSerial()

      If m_stuKey.MaxTerminal = 1 Then
        m_stuKey.SerialNumber = CLng(m_stuKey.Incrementer Mod (2 ^ LenSerial))
      Else
        Dim intMaxTermBits As Long = CLng(Math.Floor(Math.Log(m_stuKey.MaxTerminal - 1, 2)) + 1)
        Dim intBitsLeft As Long = LenSerial - intMaxTermBits
        Dim intAdjustedTerm As Long = CLng(m_stuKey.TerminalID * (2 ^ intBitsLeft))
        m_stuKey.SerialNumber = CLng((m_stuKey.Incrementer Mod (2 ^ intBitsLeft)) + intAdjustedTerm)
      End If

    End Sub

    ''' <summary>
    ''' Parses the serial.
    ''' </summary>
    Private Sub ParseSerial()

      If m_stuKey.MaxTerminal = 1 Then
        m_stuKey.TerminalID = 0
        m_stuKey.Incrementer = CLng(m_stuKey.SerialNumber Mod (2 ^ LenSerial))
      Else
        Dim intMaxTermBits As Long = CLng(Math.Floor(Math.Log(m_stuKey.MaxTerminal - 1, 2)) + 1)
        Dim intBitsLeft As Long = LenSerial - intMaxTermBits
        m_stuKey.TerminalID = m_stuKey.SerialNumber \ CLng(2 ^ intBitsLeft)
        m_stuKey.Incrementer = m_stuKey.SerialNumber Mod intBitsLeft
      End If

    End Sub

    ''' <summary>
    ''' Gets or sets the custom mod.
    ''' </summary>
    ''' <value>
    ''' The custom mod.
    ''' </value>
    ''' <exception cref="Exception">
    ''' The Custom Modification number has to be larger then zero.
    ''' or
    ''' The Custom Modification number is too large.  Largest value can be " &amp; ((2 ^ LenCustom) - 1).ToString &amp; ".
    ''' </exception>
    Public Property CustomMod() As Long

      Get

        Return m_stuKey.CustomMod

      End Get
      Set(ByVal Value As Long)

        If KeyBase = enuKeyType.Custom Then
          If Value < 1 Then Throw New Exception("The Custom Modification number has to be larger then zero.")
          If Value > (2 ^ LenCustom) - 1 Then Throw New Exception($"The Custom Modification number is too large.  Largest value can be {(2 ^ LenCustom) - 1}.")
        End If

        If m_stuKey.CustomMod.CompareTo(Value) <> 0 Then
          m_stuKey.CustomMod = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether invalid key.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if invalid key; otherwise, <c>false</c>.
    ''' </value>
    Public Property InvalidKey() As Boolean

      Get

        Return m_stuKey.InvalidKey

      End Get
      Set(ByVal Value As Boolean)

        m_stuKey.InvalidKey = Value

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets the time limit.
    ''' </summary>
    ''' <value>
    ''' The time limit.
    ''' </value>
    ''' <exception cref="Exception">
    ''' The time limit number has to be zero or larger.
    ''' or
    ''' The time limit number is too large.  Largest value can be " &amp; ((2 ^ LenTimeLimit) - 1).ToString &amp; " days.
    ''' </exception>
    Public Property TimeLimit() As Long

      Get

        If Demo Then
          Return 31
        End If
        Return m_stuKey.TimeLimit

      End Get
      Set(ByVal Value As Long)

        If KeyType <> enuKeyType.AutoUsers OrElse KeyType = enuKeyType.Users Then
          If Value < 0 Then Throw New Exception("The time limit number has to be zero or larger.")
          If Value > (2 ^ LenTimeLimit) - 1 Then Throw New Exception($"The time limit number is too large.  Largest value can be {(2 ^ LenTimeLimit) - 1} days.")
        End If

        If m_stuKey.TimeLimit.CompareTo(Value) <> 0 Then
          m_stuKey.TimeLimit = Value
          If Not m_stuKey.Parsing Then m_bolDirty = True
        End If

      End Set

    End Property

    ''' <summary>
    ''' Gets or sets a value indicating whether time extension.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if time extension; otherwise, <c>false</c>.
    ''' </value>
    Public Property TimeExtension() As Boolean

      Get

        Return m_stuKey.TimeExtension

      End Get
      Set(ByVal Value As Boolean)

        m_stuKey.TimeExtension = Value

      End Set

    End Property

    ''' <summary>
    ''' Validates the key.
    ''' </summary>
    Private Sub ValidateKey()

      m_stuKey.InvalidKey = False
      Dim intCRC As Long = GenerateCRC()
      Dim bolParity As Boolean = GenerateParity()
      If ProductType = enuProductType.Invalid Then InvalidKey = True
      If Platform = enuPlatforms.Invalid Then InvalidKey = True
      If KeyType = enuKeyType.Invalid Then InvalidKey = True
      If Modules.Length = 0 AndAlso KeyBase = enuKeyType.Product Then InvalidKey = True
      If (m_stuKey.Users < 1 OrElse m_stuKey.Users > (2 ^ LenUsers) - 1) AndAlso KeyBase = enuKeyType.Users Then InvalidKey = True
      If (m_stuKey.CustomMod < 1 OrElse m_stuKey.CustomMod > (2 ^ LenCustom) - 1) AndAlso KeyBase = enuKeyType.Custom Then InvalidKey = True
      If Not String.IsNullOrEmpty(m_strError) Then InvalidKey = True
      If m_stuKey.Parity <> bolParity Then InvalidKey = True
      If m_stuKey.CRC <> intCRC Then InvalidKey = True

    End Sub

    ''' <summary>
    ''' Gets a value indicating whether automatic activated.
    ''' </summary>
    ''' <value>
    '''   <c>true</c> if automatic activated; otherwise, <c>false</c>.
    ''' </value>
    Public ReadOnly Property AutoActivated() As Boolean

      Get

        Return KeyType = enuKeyType.AutoCustom OrElse
               KeyType = enuKeyType.AutoProduct OrElse
               KeyType = enuKeyType.AutoUsers

      End Get

    End Property

    ''' <summary>
    ''' Gets the activation array.
    ''' </summary>
    ''' <value>
    ''' The activation array.
    ''' </value>
    Friend ReadOnly Property ActivationArray() As Integer()

      Get
        ' This array is used to tell the activation key class which digits
        ' of the CD Key to use to generate the activation key.  Each one is
        ' matched up with one from the client key class to make the activation key.
        ' Each line in this array is for a different segment of the CD Key.
        Dim intReturn() As Integer =
                  {0, 6, 13, 22, 19,
                  4, 8, 24, 16, 21,
                  25, 7, 2, 10, 27,
                  14, 18, 15, 1, 20,
                  3, 12, 9, 26, 28}

        Return intReturn

      End Get

    End Property

#End Region

#Region " Public Functions ======================================================= "

    ''' <summary>
    ''' CDs the key parts comparison.
    ''' </summary>
    ''' <param name="p_stuCDKeys">The cd keys.</param>
    ''' <returns></returns>
    Public Function CDKeyPartsComparison(
        ByVal p_stuCDKeys As CLS_CDKey) As String

      Dim strReturn As String = String.Empty

      With Me
        If .Demo <> p_stuCDKeys.Demo Then
          strReturn &= " Demo not match"
        End If
        If .BPOrder <> p_stuCDKeys.BPOrder Then
          strReturn &= " BPOrder not match"
        End If
        If .Key <> p_stuCDKeys.Key Then
          strReturn &= " CDKey not match"
        End If
        If .InvalidKey <> p_stuCDKeys.InvalidKey Then
          strReturn &= " Invalid Key not match"
        End If
        If .NotForResale <> p_stuCDKeys.NotForResale Then
          strReturn &= " NotForResale not match"
        End If
        If GetModuleFlags(.Modules) <> GetModuleFlags(p_stuCDKeys.Modules) Then
          strReturn &= " Modules do not match"
        End If
        If .Platform <> p_stuCDKeys.Platform Then
          strReturn &= " Platform not match"
        End If
        If .SerialNumber.CompareTo(p_stuCDKeys.SerialNumber) <> 0 Then
          strReturn &= " SerialNumber not match"
        End If
        If .Source <> p_stuCDKeys.Source Then
          strReturn &= " BPOrder not match"
        End If
        If .Upgrade <> p_stuCDKeys.Upgrade Then
          strReturn &= " Upgrade not match"
        End If
        If .Users <> p_stuCDKeys.Users Then
          strReturn &= " Users not match"
        End If
        If .Version <> p_stuCDKeys.Version Then
          strReturn &= " Version not match"
        End If
      End With

      Return strReturn

    End Function

    ''' <summary>
    ''' Gets the cd key parts string labeled.
    ''' </summary>
    ''' <returns></returns>
    Public Function GetCDKeyPartsStringLabeled() As String

      Dim strReturn As StringWriter = Nothing
      Dim s As String
      Try
        strReturn = New StringWriter()
        With Me
          strReturn.WriteLine("CDKey = {0}", .Key)
          strReturn.WriteLine("KeyType = {0}", .KeyType)
          strReturn.WriteLine("Platform = {0}", .Platform)
          strReturn.WriteLine("Version = {0}", .Version)
          strReturn.WriteLine("Users = {0}", .Users)
          strReturn.WriteLine("NotForResale = {0}", .NotForResale)
          strReturn.WriteLine("BPOrder = {0}", .BPOrder)
          strReturn.WriteLine("Source = {0}", .Source)
          strReturn.WriteLine("Upgrade = {0}", .Upgrade)
          strReturn.WriteLine("Demo = {0}", .Demo)
          strReturn.WriteLine("Fund = {0}", .Fund)
          strReturn.WriteLine("SerialNumber = {0}", .SerialNumber)
          strReturn.WriteLine("CustomMod = {0}", .CustomMod)
          strReturn.WriteLine("InvalidKey = {0}", .InvalidKey)
        End With
        s = strReturn.ToString()
      Finally
        If strReturn IsNot Nothing Then
          strReturn.Close()
          strReturn = Nothing
        End If
      End Try

      Return s

    End Function

    ''' <summary>
    ''' Gets the cd key XML information.
    ''' </summary>
    ''' <param name="p_bolEncrypt">if set to <c>true</c> encrypt.</param>
    ''' <returns></returns>
    Public Function GetCDKeyXMLInfo(
        ByVal p_bolEncrypt As Boolean) As String

      Dim strSpace As String = "   "
      Dim strReturn As String = "<root />"

      If Not InvalidKey Then
        Dim strXML As StringWriter = Nothing
        Try
          strXML = New StringWriter()
          With strXML
            .WriteLine("<root>")
            .WriteLine(BuildXMLLine(strSpace, "CDKey", Key))
            .WriteLine(BuildXMLLine(strSpace, "KeyType", KeyType.ToString))
            .WriteLine(BuildXMLLine(strSpace, "Platform", Platform.ToString))
            .WriteLine(BuildXMLLine(strSpace, "Version", Version.ToString))
            .WriteLine(BuildXMLLine(strSpace, "Users", Users.ToString))
            .WriteLine(BuildXMLLine(strSpace, "NotForResale", BooleanToInteger(NotForResale).ToString))
            .WriteLine(BuildXMLLine(strSpace, "BPOrder", BooleanToInteger(BPOrder).ToString))
            .WriteLine(BuildXMLLine(strSpace, "Source", BooleanToInteger(Source).ToString))
            .WriteLine(BuildXMLLine(strSpace, "Upgrade", BooleanToInteger(Upgrade).ToString))
            .WriteLine(BuildXMLLine(strSpace, "Demo", BooleanToInteger(Demo).ToString))
            .WriteLine(BuildXMLLine(strSpace, "Fund", CStr(BooleanToInteger(Fund))))
            .WriteLine(BuildXMLLine(strSpace, "SerialNumber", SerialNumber.ToString))
            .WriteLine(BuildXMLLine(strSpace, "CustomMod", CustomMod.ToString))
            .WriteLine("</root>")
          End With
          strReturn = strXML.ToString()
        Finally
          If strXML IsNot Nothing Then
            strXML.Close()
            strXML = Nothing
          End If
        End Try
      End If

      If p_bolEncrypt Then
        Return Encrypt(strReturn, parg & park & pard & parq & parh)
      End If

      Return strReturn

    End Function

    ''' <summary>
    ''' Returns true if ... is valid.
    ''' </summary>
    ''' <returns>
    '''   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
    ''' </returns>
    Public Function IsValid() As Boolean

      Return Not InvalidKey

    End Function

    ''' <summary>
    ''' Converts to string.
    ''' </summary>
    ''' <returns>
    ''' A <see cref="String" /> that represents this instance.
    ''' </returns>
    Public Shadows Function ToString() As String

      Dim strReturn As String
      strReturn = $"Application: {vbTab}{vbTab}{vbTab}{GetPlatformDescription(Platform)}{vbCrLf}KeyType: {vbTab}{vbTab}{vbTab}{GetKeyType(KeyType)}{vbCrLf}"
      Select Case KeyBase
        Case enuKeyType.Product
          strReturn &= $"Modules: {vbTab}{vbTab}{vbTab}{KeyDescription}{vbCrLf}Time Lock Key: {vbTab}{vbTab}{Demo}{vbCrLf}Product Type: {vbTab}{vbTab}{ProductType()}{vbCrLf}Version Number: {vbTab}{vbTab}{GetVersionNumber(Version)}{vbCrLf}"
          If TimeLimit > 0 Then
            strReturn &= $"Expires in: {vbTab}{vbTab}{vbTab}{TimeLimit} Days{vbCrLf}"
          End If
          strReturn &= $"BP Order: {vbTab}{vbTab}{vbTab}{BPOrder}{vbCrLf}Source Code: {vbTab}{vbTab}{Source}{vbCrLf}Upgrade: {vbTab}{vbTab}{vbTab}{Upgrade}{vbCrLf}Not For Resale: {vbTab}{vbTab}{NotForResale}{vbCrLf}"
        Case enuKeyType.Users
          strReturn &= $"Users: {vbTab}{vbTab}{vbTab}{UserDescription}{vbCrLf}Product Type: {vbTab}{vbTab}{ProductType()}{vbCrLf}BP Order: {vbTab}{vbTab}{vbTab}{BPOrder}{vbCrLf}Upgrade: {vbTab}{vbTab}{vbTab}{Upgrade}{vbCrLf}"
        Case enuKeyType.Custom
          strReturn &= $"Custom Mod Number: {vbTab}{CustomMod}{vbCrLf}Time Lock Key: {vbTab}{vbTab}{Demo}{vbCrLf}Version Number: {vbTab}{vbTab}{GetVersionNumber(Version)}{vbCrLf}"
          If TimeLimit > 0 Then
            strReturn &= $"Expires in: {vbTab}{vbTab}{vbTab}{TimeLimit} Days{vbCrLf}"
          End If
          strReturn &= $"BP Order: {vbTab}{vbTab}{vbTab}{BPOrder}{vbCrLf}Upgrade: {vbTab}{vbTab}{vbTab}{Upgrade}{vbCrLf}Not For Resale: {vbTab}{vbTab}{NotForResale}{vbCrLf}"
      End Select
      strReturn &= $"Fund: {vbTab}{vbTab}{vbTab}{vbTab}{Fund}{vbCrLf}Serial Number: {vbTab}{vbTab}{SerialNumber}"

      Return strReturn

    End Function

#End Region

#Region " Private Functions ====================================================== "

#Region " Key Info =============================================================== "

    ''' <summary>
    ''' Get16s the bit key information.
    ''' </summary>
    ''' <returns></returns>
    Private Function Get16BitKeyInfo() As Integer()

      Dim intPos() As Integer = {13, 25, 29, 7, 23,
                                 17, 16, 26, 8, 1,
                                 22, 10, 21, 9, 3,
                                 19, 4, 27, 11, 5,
                                 15, 2, 20, 28, 14}

      Return intPos

    End Function

    ''' <summary>
    ''' Get32s the bit key information.
    ''' </summary>
    ''' <returns></returns>
    Private Function Get32BitKeyInfo() As Integer()

      Dim intPos() As Integer = {13, 25, 29, 23, 8,
                                 21, 11, 3, 15, 2,
                                 17, 27, 19, 9, 5,
                                 1, 16, 10, 20, 26,
                                 14, 4, 22, 7, 28}

      Return intPos

    End Function

    ''' <summary>
    ''' Get64s the bit key information.
    ''' </summary>
    ''' <returns></returns>
    Private Function Get64BitKeyInfo() As Integer()

      Dim intPos() As Integer = {13, 25, 29, 27, 19,
                                 9, 5, 1, 16, 10,
                                 20, 26, 14, 4, 22,
                                 7, 28, 23, 8, 21,
                                 11, 3, 15, 2, 17}

      Return intPos

    End Function

#End Region

#Region " Key Rotate ============================================================= "

    ''' <summary>
    ''' Get16s the bit key rotate.
    ''' </summary>
    ''' <returns></returns>
    Private Function Get16BitKeyRotate() As Integer()

      Dim intPos() As Integer = {-2, 8, 5, -8, 3,
                                  4, -10, -1, -2, -4,
                                 10, 6, 4, -2, 5,
                                 -4, -3, -1, 9, -8,
                                  3, -3, -8, -6, -8}

      Return intPos

    End Function

    ''' <summary>
    ''' Get32s the bit key rotate.
    ''' </summary>
    ''' <returns></returns>
    Private Function Get32BitKeyRotate() As Integer()

      Dim intPos() As Integer = {-2, 9, 10, 7, -2,
                                 6, 10, -5, -7, 4,
                               -10, -3, 4, -8, -8,
                                -4, -2, 1, -4, -4,
                                 6, -4, -2, -2, 7}

      Return intPos

    End Function

    ''' <summary>
    ''' Get64s the bit key rotate.
    ''' </summary>
    ''' <returns></returns>
    Private Function Get64BitKeyRotate() As Integer()

      Dim intPos() As Integer = {-2, -1, 5, 5, 6,
                                 -6, -1, 7, 7, -2,
                                 -1, -3, -5, 6, 1,
                                 1, 3, -8, 4, -5,
                                 -9, -4, 9, -3, 3}

      Return intPos

    End Function

#End Region

#Region " Bit Manipulation ======================================================= "

    ''' <summary>
    ''' Sets the bits.
    ''' </summary>
    ''' <param name="p_intStart">The starting bit.</param>
    ''' <param name="p_intLength">Length of the bits.</param>
    ''' <param name="p_intValue">The value to put in.</param>
    Private Sub SetBits(
        ByRef p_intStart As Integer,
        ByVal p_intLength As Integer,
        ByVal p_intValue As Long)

      Dim strValue As String = String.Empty
      Dim intValue As Long = p_intValue

      ' Lets say that we have a number of 375 we want in 16 bits, we want to get a binary string
      ' that looks like 0000000101110111 to do this, we take 375 and divide it by 2 and get the
      ' remainder (Mod) which will give you a 1 in this case.  You then take the left over part,
      ' 374, and divide by 2 giving you 187.  You then continue this process adding the new
      ' remainders to the front of the string.
      Do Until intValue <= 0
        strValue = $"{intValue Mod 2}{strValue}"
        intValue = intValue \ 2
      Loop

      ' Make sure we are the right length.  Pad with zeros and trim as well. PadRight does not make the string smaller.
      strValue = strValue.PadLeft(p_intLength, CChar("0"))
      strValue = strValue.Substring(strValue.Length - p_intLength, p_intLength)
      For intCntr As Integer = p_intStart To p_intLength + p_intStart - 1
        m_strKeyArray(intCntr) = strValue.Chars(intCntr - p_intStart)
      Next

      ' Return the value of where to start the next one.
      p_intStart += p_intLength

    End Sub

    ''' <summary>
    ''' Sets the module bits.
    ''' </summary>
    ''' <param name="p_intStart">The p int start.</param>
    Private Sub SetModuleBits(
        ByRef p_intStart As Integer)

      Dim intBits As Integer = LenModules
      Dim strValue As String = m_stuKey.Modules

      ' Make sure we are the right length.
      strValue = strValue.PadRight(intBits, CChar("0")).Substring(0, intBits)
      For intCntr As Integer = p_intStart To intBits + p_intStart - 1
        m_strKeyArray(intCntr) = strValue.Chars(intCntr - p_intStart)
      Next

      ' Return the value of where to start the next one.
      p_intStart += intBits

    End Sub

    ''' <summary>
    ''' Gets the bits.
    ''' </summary>
    ''' <param name="p_intStart">The starting position.</param>
    ''' <param name="p_intLength">Length of the bits to get.</param>
    ''' <returns></returns>
    Private Function GetBits(
        ByRef p_intStart As Integer,
        ByVal p_intLength As Integer) As Long

      Dim intValue As Long = 0

      ' This converts the set of bits to a number.  To do this we do this:
      ' p_intLength = 5
      ' p_intStart = 4

      ' we get the following as we loop through:
      ' intCntr = 4 - Calc to 2^(5-(4-4)-1) = 2^(5-(0)-1) = 16
      ' intCntr = 5 - Calc to 2^(5-(5-4)-1) = 2^(5-(1)-1) =  8
      ' intCntr = 6 - Calc to 2^(5-(6-4)-1) = 2^(5-(2)-1) =  4
      ' intCntr = 7 - Calc to 2^(5-(7-4)-1) = 2^(5-(3)-1) =  2
      ' intCntr = 8 - Calc to 2^(5-(8-4)-1) = 2^(5-(4)-1) =  1

      ' We multiply each one by the 1 or 0 to give us the number

      For intCntr As Integer = p_intStart To p_intLength + p_intStart - 1
        intValue += CLng(CLng(m_strKeyArray(intCntr).ToString) * (2 ^ (p_intLength - (intCntr - p_intStart) - 1)))
      Next

      p_intStart += p_intLength

      Return intValue

    End Function

    ''' <summary>
    ''' Gets the module bits.
    ''' </summary>
    ''' <param name="p_intStart">The starting position.</param>
    ''' <returns></returns>
    Private Function GetModuleBits(
        ByRef p_intStart As Integer) As String

      Dim intBits As Integer = LenModules
      Dim strValue As String = String.Empty

      ' Make sure we are the right length.
      strValue = strValue.PadLeft(intBits, CChar("0"))
      For intCntr As Integer = p_intStart To intBits + p_intStart - 1
        Mid(strValue, intCntr - p_intStart + 1, 1) = m_strKeyArray(intCntr).ToString
      Next

      ' Return the value of where to start the next one.
      p_intStart += intBits

      Return strValue

    End Function

    ''' <summary>
    ''' Generates the CRC.
    ''' </summary>
    ''' <returns></returns>
    Private Function GenerateCRC() As Long

      Const intBits As Integer = LenCRC

      Dim intReturn As Long = 0
      Dim intPos As Integer = 0

      ' Get all of the parts that are a full 16 bits
      For intCntr As Integer = 0 To CInt(((124 - intBits) / intBits) - 1)
        intReturn += GetBits(intPos, intBits)
        ' Make sure that the value rolls back past zero when over (2^16)-1
        If intReturn > (2 ^ intBits) - 1 Then
          intReturn -= CLng(2 ^ intBits)
        End If
      Next
      ' Get the bits that are left in the process
      intReturn += GetBits(intPos, (124 - intBits) - intPos)
      ' Make sure that the value rolls back past zero when over (2^16)-1
      If intReturn > (2 ^ intBits) - 1 Then
        intReturn -= CLng(2 ^ intBits)
      End If

      Return intReturn

    End Function

    ''' <summary>
    ''' Generates the parity.
    ''' </summary>
    ''' <returns></returns>
    Private Function GenerateParity() As Boolean

      ' Returns true if the number of bits are odd.

      Dim intCntr As Integer = 0
      For Each cItem As Char In m_strKeyArray
        If cItem.ToString = "1" Then intCntr += 1
      Next

      If m_strKeyArray(m_strKeyArray.Length - 1).ToString = "1" Then intCntr -= 1

      Return intCntr Mod 2 = 1

    End Function

    ''' <summary>
    ''' Generates the key values.
    ''' </summary>
    ''' <param name="p_intRotates">The rotates.</param>
    ''' <returns></returns>
    Private Function GenerateKeyValues(
        ByRef p_intRotates As Integer()) As String

      Dim strReturn As String = String.Empty
      Dim intPos As Integer = 0

      Do While intPos < m_strKeyArray.Length
        Dim intChar As Long = GetBits(intPos, 5)
        intChar += p_intRotates((intPos - 5) \ 5)
        If intChar > ValidCharacters.Length - 1 Then intChar -= ValidCharacters.Length
        If intChar < 0 Then intChar += ValidCharacters.Length
        strReturn &= Mid(ValidCharacters, CInt(intChar + 1), 1)
      Loop

      Return strReturn

    End Function

    ''' <summary>
    ''' Recovers the key bits.
    ''' </summary>
    ''' <param name="p_intRotates">The rotates.</param>
    ''' <param name="p_strKey">The key.</param>
    Private Sub RecoverKeyBits(
        ByRef p_intRotates As Integer(),
        ByVal p_strKey As String)

      Dim strKey As String = p_strKey.Replace("-", "")
      Dim intPos As Integer = 0

      ' Clears the array and makes sure it is the right size
      ReDim m_strKeyArray((strKey.Length * 5) - 1)

      Do While intPos < m_strKeyArray.Length
        Dim intChar As Long = InStr(ValidCharacters, Mid(strKey, (intPos \ 5) + 1, 1)) - 1
        intChar -= p_intRotates(intPos \ 5)
        If intChar > ValidCharacters.Length - 1 Then intChar -= ValidCharacters.Length
        If intChar < 0 Then intChar += ValidCharacters.Length
        SetBits(intPos, 5, intChar)
      Loop

    End Sub

    ''' <summary>
    ''' Fills the key.
    ''' </summary>
    ''' <param name="p_strData">The data.</param>
    ''' <param name="p_intPlaces">The places to put each part.</param>
    ''' <returns></returns>
    Private Function FillKey(
        ByVal p_strData As String,
        ByVal p_intPlaces() As Integer) As String

      Dim strKey As String = "xxxxx-xxxxx-xxxxx-xxxxx-xxxxx"

      For intcntr As Integer = 0 To p_intPlaces.Length - 1
        Mid(strKey, p_intPlaces(intcntr), 1) = Mid(p_strData, intcntr + 1, 1)
      Next

      Return strKey

    End Function

    ''' <summary>
    ''' Gets the data.
    ''' </summary>
    ''' <param name="p_intPlaces">The places to put each part.</param>
    ''' <returns></returns>
    Private Function GetData(
        ByVal p_intPlaces() As Integer) As String

      Return GetData(Key, p_intPlaces)

    End Function

    ''' <summary>
    ''' Gets the data.
    ''' </summary>
    ''' <param name="p_strKey">The key.</param>
    ''' <param name="p_intPlaces">The places to put each part.</param>
    ''' <returns></returns>
    Private Function GetData(
        ByVal p_strKey As String,
        ByVal p_intPlaces() As Integer) As String

      Dim strReturn As String = String.Empty

      For intcntr As Integer = 0 To p_intPlaces.Length - 1
        strReturn &= Mid(p_strKey, p_intPlaces(intcntr), 1)
      Next

      Return strReturn

    End Function

#End Region

    ''' <summary>
    ''' Generates the key.
    ''' </summary>
    ''' <returns></returns>
    Private Function GenerateKey() As String

      Select Case Platform
        Case enuPlatforms.CMS_16Bit
          Return FillKey(GenerateData(), Get16BitKeyInfo())
        Case enuPlatforms.CMS_32Bit
          Return FillKey(GenerateData(), Get32BitKeyInfo())
        Case enuPlatforms.CMS_64Bit
          Return FillKey(GenerateData(), Get64BitKeyInfo())
        Case Else
          Return String.Empty
      End Select

    End Function

    ''' <summary>
    ''' Generates the data.
    ''' </summary>
    ''' <returns></returns>
    Private Function GenerateData() As String

      For intCntr As Integer = 0 To m_strKeyArray.Length - 1
        m_strKeyArray(intCntr) = CChar(CStr(CInt(Rnd())))
      Next

      Dim intPos As Integer = 0

      ' Platform                  4
      SetBits(intPos, 4, Platform)
      ' Key Type                  3
      SetBits(intPos, 3, KeyType)

      Select Case KeyBase
        Case enuKeyType.Product
          ' Modules                  32
          SetModuleBits(intPos)
          ' Not For Resale	          1
          SetBits(intPos, 1, If(NotForResale, 1, 0))
          ' Business Partner	        1
          SetBits(intPos, 1, If(BPOrder, 1, 0))
          ' Source Code License       1
          SetBits(intPos, 1, If(Source AndAlso BPOrder, 1, 0))
          ' For Profit/Not For Profit	1
          SetBits(intPos, 1, If(Fund, 1, 0))
          ' Serial Number	           24
          SetBits(intPos, LenSerial, SerialNumber)
          ' Version                  12
          SetBits(intPos, LenVersion, Version)
          ' Upgrade                   1
          SetBits(intPos, 1, If(Upgrade, 1, 0))
          ' Demo Mode	                1
          SetBits(intPos, 1, If(Demo, 1, 0))
          ' Time Limit	             12
          SetBits(intPos, LenTimeLimit, TimeLimit)
          ' Time Extension            1
          SetBits(intPos, 1, If(TimeExtension, 1, 0))
        Case enuKeyType.Users
          'Number of Users	         32
          SetBits(intPos, LenUsers, Users)
          ' Business Partner	        1
          SetBits(intPos, 1, If(BPOrder, 1, 0))
          ' For Profit/Not For Profit	1
          SetBits(intPos, 1, If(Fund, 1, 0))
          ' Serial Number	           24
          SetBits(intPos, LenSerial, SerialNumber)
          ' Upgrade                   1
          SetBits(intPos, 1, If(Upgrade, 1, 0))
        Case enuKeyType.Custom
          'Custom Modification Number	24
          SetBits(intPos, LenCustom, CustomMod)
          ' Not For Resale	          1
          SetBits(intPos, 1, If(NotForResale, 1, 0))
          ' Business Partner	        1
          SetBits(intPos, 1, If(BPOrder, 1, 0))
          ' For Profit/Not For Profit	1
          SetBits(intPos, 1, If(Fund, 1, 0))
          ' Upgrade                   1
          SetBits(intPos, 1, If(Upgrade, 1, 0))
          ' Demo Mode	                1
          SetBits(intPos, 1, If(Demo, 1, 0))
          ' Serial Number	           24
          SetBits(intPos, LenSerial, SerialNumber)
          ' Version                  12
          SetBits(intPos, LenVersion, Version)
          ' Time Limit	             12
          SetBits(intPos, LenTimeLimit, TimeLimit)
      End Select

      ' CRC Check	               12
      m_stuKey.CRC = GenerateCRC()
      SetBits(124 - LenCRC, LenCRC, m_stuKey.CRC)
      ' Parity check	            1
      m_stuKey.Parity = GenerateParity()
      SetBits(124, 1, If(m_stuKey.Parity, 1, 0))

      ValidateKey()

      Select Case Platform
        Case enuPlatforms.CMS_16Bit
          Return GenerateKeyValues(Get16BitKeyRotate())
        Case enuPlatforms.CMS_32Bit
          Return GenerateKeyValues(Get32BitKeyRotate())
        Case enuPlatforms.CMS_64Bit
          Return GenerateKeyValues(Get64BitKeyRotate())
        Case Else
          Return String.Empty
      End Select

    End Function

    ''' <summary>
    ''' Parses the key.
    ''' </summary>
    Private Sub ParseKey()

      m_stuKey.Parsing = True
      m_strError = String.Empty

      Try
        RecoverKeyBits(Get32BitKeyRotate(), GetData(Get32BitKeyInfo()))

        Dim enmPlatform As enuPlatforms = CType(GetBits(0, 4), enuPlatforms)

        Select Case enmPlatform
          Case enuPlatforms.CMS_16Bit
            RecoverKeyBits(Get16BitKeyRotate(), GetData(Get16BitKeyInfo()))
          Case enuPlatforms.CMS_32Bit
            '        RecoverKeyBits(Get32BitKeyRotate(), GetData(p_intPlaces)) ' Already done.  Change to here and above when 64 bit is more likely
          Case enuPlatforms.CMS_64Bit
            RecoverKeyBits(Get64BitKeyRotate(), GetData(Get64BitKeyInfo()))
        End Select

        Dim intPos As Integer = 0

        ' Platform                  4
        Platform = CType(GetBits(intPos, 4), enuPlatforms)
        ' Key Type                  3
        KeyType = CType(GetBits(intPos, 3), enuKeyType)
        Select Case KeyBase
          Case enuKeyType.Product
            ' Modules                  32
            m_stuKey.Modules = GetModuleBits(intPos)
            ' Not For Resale	          1
            NotForResale = GetBits(intPos, 1) = 1
            ' Business Partner	        1
            BPOrder = GetBits(intPos, 1) = 1
            ' Source Code License       1
            Source = GetBits(intPos, 1) = 1
            ' For Profit/Not For Profit	1
            Fund = GetBits(intPos, 1) = 1
            If Not Fund Then ProductType = enuProductType.Act
            ' Serial Number	           24
            SerialNumber = GetBits(intPos, LenSerial)
            ' Version                  12
            Version = CInt(GetBits(intPos, LenVersion))
            ' Upgrade                   1
            Upgrade = GetBits(intPos, 1) = 1
            ' Demo Mode	                1
            Demo = GetBits(intPos, 1) = 1
            ' Time Limit	             16
            TimeLimit = GetBits(intPos, LenTimeLimit)
            ' Time Extension            1
            TimeExtension = GetBits(intPos, 1) = 1
          Case enuKeyType.Users
            'Number of Users	         32
            Users = GetBits(intPos, LenUsers)
            ' Business Partner	        1
            BPOrder = GetBits(intPos, 1) = 1
            ' For Profit/Not For Profit	1
            Fund = GetBits(intPos, 1) = 1
            If Not Fund Then ProductType = enuProductType.Act
            ' Serial Number	           24
            SerialNumber = GetBits(intPos, LenSerial)
            ' Upgrade                   1
            Upgrade = GetBits(intPos, 1) = 1
          Case enuKeyType.Custom
            'Custom Modification Number	32
            CustomMod = GetBits(intPos, LenCustom)
            ' Not For Resale	          1
            NotForResale = GetBits(intPos, 1) = 1
            ' Business Partner	        1
            BPOrder = GetBits(intPos, 1) = 1
            ' For Profit/Not For Profit	1
            Fund = GetBits(intPos, 1) = 1
            If Not Fund Then ProductType = enuProductType.Act
            ' Upgrade                   1
            Upgrade = GetBits(intPos, 1) = 1
            ' Demo Mode	                1
            Demo = GetBits(intPos, 1) = 1
            ' Serial Number	           24
            SerialNumber = GetBits(intPos, LenSerial)
            ' Version                  12
            Version = CInt(GetBits(intPos, LenVersion))
            ' Time Limit	             16
            TimeLimit = GetBits(intPos, LenTimeLimit)
        End Select

        ' CRC Check	               16
        m_stuKey.CRC = GetBits(124 - LenCRC, LenCRC)
        ' Parity check	            1
        m_stuKey.Parity = GetBits(124, 1) = 1
      Catch ex As Exception
        m_strError = ex.Message
        InvalidKey = True
      End Try

      m_stuKey.Parsing = False

      ValidateKey()

    End Sub

    ''' <summary>
    ''' Gets the type of the product.
    ''' </summary>
    ''' <returns></returns>
    Private Function GetProductType() As enuProductType

      Return If(Fund, enuProductType.Fund, enuProductType.Act)

    End Function

#End Region

#Region " Shared operators ======================================================= "

    ''' <summary>
    ''' Implements the operator =.
    ''' </summary>
    ''' <param name="leftKey">The first key.</param>
    ''' <param name="rightKey">The second key.</param>
    ''' <returns>
    ''' The result of the operator.
    ''' </returns>
    Public Shared Operator =(
        ByVal leftKey As CLS_CDKey,
        ByVal rightKey As CLS_CDKey) As Boolean

      If leftKey Is Nothing AndAlso rightKey Is Nothing Then Return True
      If leftKey Is Nothing OrElse rightKey Is Nothing Then Return False
      If ReferenceEquals(leftKey, rightKey) Then Return True
      If leftKey.Key = rightKey.Key Then Return True
      If leftKey.InvalidKey Or rightKey.InvalidKey Then Return False
      If leftKey.AutoActivated <> rightKey.AutoActivated Then Return False
      If leftKey.Platform <> rightKey.Platform Then Return False
      If leftKey.KeyType <> rightKey.KeyType Then Return False
      Select Case leftKey.KeyBase
        Case enuKeyType.Product
          If leftKey.Source <> rightKey.Source Then Return False
          If leftKey.NotForResale <> rightKey.NotForResale Then Return False
          If leftKey.Version <> rightKey.Version Then Return False
          If leftKey.Demo <> rightKey.Demo Then Return False
          If leftKey.TimeLimit <> rightKey.TimeLimit Then Return False
          If leftKey.TimeExtension <> rightKey.TimeExtension Then Return False
        Case enuKeyType.Users
          If leftKey.Users <> rightKey.Users Then Return False
        Case enuKeyType.Custom
          If leftKey.CustomMod <> rightKey.CustomMod Then Return False
          If leftKey.NotForResale <> rightKey.NotForResale Then Return False
          If leftKey.Demo <> rightKey.Demo Then Return False
          If leftKey.Version <> rightKey.Version Then Return False
          If leftKey.TimeLimit <> rightKey.TimeLimit Then Return False
      End Select
      If leftKey.BPOrder <> rightKey.BPOrder Then Return False
      If leftKey.Fund <> rightKey.Fund Then Return False
      If leftKey.Upgrade <> rightKey.Upgrade Then Return False
      If leftKey.SerialNumber <> rightKey.SerialNumber Then Return False
      If Not (leftKey.Modules Is Nothing AndAlso rightKey.Modules Is Nothing) Then ' If at least one of the arrays is defined
        If leftKey.Modules Is Nothing OrElse rightKey.Modules Is Nothing Then Return False ' If one is defined and the other is not
        If leftKey.Modules.Length <> rightKey.Modules.Length Then Return False ' If the lengths are different
        If leftKey.Modules.Except(rightKey.Modules).Any() Then Return False ' If the contents are different
      End If
      Return True

    End Operator

    ''' <summary>
    ''' Implements the operator &lt;&gt;.
    ''' </summary>
    ''' <param name="leftKey">The first key.</param>
    ''' <param name="rightKey">The second key.</param>
    ''' <returns>
    ''' The result of the operator.
    ''' </returns>
    Public Shared Operator <>(
        ByVal leftKey As CLS_CDKey,
        ByVal rightKey As CLS_CDKey) As Boolean

      Return Not leftKey = rightKey

    End Operator

    ''' <summary>
    ''' Determines whether the specified <see cref="System.Object" />, is equal to this instance.
    ''' </summary>
    ''' <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
    ''' <returns>
    '''   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
    ''' </returns>
    Public Overrides Function Equals(obj As Object) As Boolean

      If TypeOf obj Is CLS_CDKey Then
        Dim otherKey As CLS_CDKey = CType(obj, CLS_CDKey)
        Return Me = otherKey
      End If

      Return False

    End Function

    ''' <summary>
    ''' Returns a hash code for this instance.
    ''' </summary>
    ''' <returns>
    ''' A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
    ''' </returns>
    Public Overrides Function GetHashCode() As Integer

      Return Key.GetHashCode()

    End Function

#End Region

  End Class

End Namespace

#End Region
