namespace BizHawk.Client.Common
{
	public enum ELuaEngine
	{
		/// <remarks>Don't change this member's ordinal (don't reorder) without changing <c>BizHawk.Client.EmuHawk.Program.CurrentDomain_AssemblyResolve</c></remarks>
		LuaPlusLuaInterface,
		NLuaPlusKopiLua
	}

	public enum ESoundOutputMethod
	{
		DirectSound, XAudio2, OpenAL, Dummy
	}

	public enum EDispManagerAR
	{
		None,
		System,

		// actually, custom SIZE (fixme on major release)
		Custom,
		CustomRatio
	}

	public enum EHostInputMethod
	{
		OpenTK = 0,
		DirectInput = 1
	}
}
