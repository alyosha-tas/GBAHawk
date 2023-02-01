using System;
using System.Globalization;

using BizHawk.Emulation.Common;

namespace BizHawk.Client.Common
{
	/// <summary>
	/// This class holds a converter for BizHawk SystemId (which is a simple <see cref="string"/>
	/// It allows you to convert it to a <see cref="CoreSystem"/> value and vice versa
	/// </summary>
	/// <remarks>I made it this way just in case one day we need it for WPF (DependencyProperty binding). Just uncomment :IValueConverter implementation
	/// I didn't implemented it because of mono compatibility
	/// </remarks>
	public sealed class BizHawkSystemIdToEnumConverter //:IValueConverter
	{
		/// <summary>
		/// Convert BizHawk SystemId <see cref="string"/> to <see cref="CoreSystem"/> value
		/// </summary>
		/// <param name="value"><see cref="string"/> you want to convert</param>
		/// <param name="targetType">The type of the binding target property</param>
		/// <param name="parameter">The converter parameter to use; null in our case</param>
		/// <param name="cultureInfo">The culture to use in the converter</param>
		/// <returns>A <see cref="CoreSystem"/> that is equivalent to BizHawk SystemId <see cref="string"/></returns>
		/// <exception cref="IndexOutOfRangeException">Thrown when SystemId hasn't been found</exception>
		public object Convert(object value, Type targetType, object parameter, CultureInfo cultureInfo)
		{
			return (string) value switch
			{
				VSystemID.Raw.GBL => CoreSystem.GameBoyLink,
				VSystemID.Raw.GB => CoreSystem.GameBoy,
				VSystemID.Raw.GBA => CoreSystem.GameBoyAdvance,
				VSystemID.Raw.NULL => CoreSystem.Null,
				_ => throw new IndexOutOfRangeException($"{value} is missing in convert list")
			};
		}


		/// <summary>
		/// Convert BizHawk SystemId <see cref="string"/> to <see cref="CoreSystem"/> value
		/// </summary>
		/// <param name="value"><see cref="string"/> you want to convert</param>
		/// <returns>A <see cref="CoreSystem"/> that is equivalent to BizHawk SystemId <see cref="string"/></returns>
		/// <exception cref="IndexOutOfRangeException">Thrown when SystemId hasn't been found</exception>
		public CoreSystem Convert(string value)
		{
			return (CoreSystem)Convert(value, null, null, CultureInfo.CurrentCulture);
		}


		/// <summary>
		/// Convert a <see cref="CoreSystem"/> value to BizHawk SystemId <see cref="string"/>
		/// </summary>
		/// <param name="value"><see cref="CoreSystem"/> you want to convert</param>
		/// <param name="targetType">The type of the binding target property</param>
		/// <param name="parameter">The converter parameter to use; null in our case</param>
		/// <param name="cultureInfo">The culture to use in the converter</param>
		/// <returns>A <see cref="string"/> that is used by BizHawk SystemId</returns>
		/// <exception cref="IndexOutOfRangeException">Thrown when <see cref="CoreSystem"/> hasn't been found</exception>
		public object ConvertBack(object value, Type targetType, object parameter, CultureInfo cultureInfo)
		{
			return (CoreSystem) value switch
			{
				CoreSystem.GameBoyLink => VSystemID.Raw.GBL,
				CoreSystem.GameBoy => VSystemID.Raw.GB,
				CoreSystem.GameBoyAdvance => VSystemID.Raw.GBA,
				CoreSystem.Null => VSystemID.Raw.NULL,
				_ => throw new IndexOutOfRangeException($"{value} is missing in convert list")
			};
		}


		/// <summary>
		/// Convert a <see cref="CoreSystem"/> value to BizHawk SystemId <see cref="string"/>
		/// </summary>
		/// <param name="value"><see cref="CoreSystem"/> you want to convert</param>
		/// <returns>A <see cref="string"/> that is used by BizHawk SystemId</returns>
		/// <exception cref="IndexOutOfRangeException">Thrown when <see cref="CoreSystem"/> hasn't been found</exception>
		public string ConvertBack(CoreSystem value)
		{
			return (string)ConvertBack(value, null, null, CultureInfo.CurrentCulture);
		}
	}
}
