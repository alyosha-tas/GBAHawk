// GBHawk.cpp : Defines the exported functions for the DLL.

#include "GBHawk.h"
#include "Core.h"
#include "LinkCore.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace GBHawk;

#pragma region Core
// Create pointer to a core instance
GBHawk_EXPORT GBCore* GB_create()
{
	return new GBCore();
}

// free the memory from the core pointer
GBHawk_EXPORT void GB_destroy(GBCore* p)
{
	delete p->GB.Cart_RAM;

	std::free(p);
}

// load bios and basic into the core
GBHawk_EXPORT void GB_load_bios(GBCore* p, uint8_t* bios)
{
	p->Load_BIOS(bios);
}

// load a rom into the core
GBHawk_EXPORT void GB_load(GBCore* p, uint8_t* rom, uint32_t size, uint32_t mapper, uint64_t datetime, bool rtc_functional, int16_t EEPROM_offset, uint16_t flash_type_64_value,
							uint16_t flash_type_128_value, int16_t flash_write_offset, int32_t flash_sector_offset, int32_t flash_chip_offset, bool is_GBP)
{
	p->Load_ROM(rom, size, mapper, datetime, rtc_functional, EEPROM_offset, flash_type_64_value, flash_type_128_value, flash_write_offset, flash_sector_offset, flash_chip_offset, is_GBP);
}

// Create a default SRAM
GBHawk_EXPORT void GB_create_SRAM(GBCore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
GBHawk_EXPORT void GB_load_SRAM(GBCore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

GBHawk_EXPORT void GB_load_Palette(GBCore* p, bool palette)
{
	p->Set_Palette(palette);
}

GBHawk_EXPORT void GB_Sync_Domain_VBL(GBCore* p, bool on_vbl)
{
	p->Sync_Domain_VBL(on_vbl);
}

// reset the system
GBHawk_EXPORT void GB_Hard_Reset(GBCore* p)
{
	p->Hard_Reset();
}

// advance a frame
GBHawk_EXPORT bool GB_frame_advance(GBCore* p, uint8_t ctrl1, uint16_t accx, uint16_t accy, bool render, bool sound)
{
	return p->FrameAdvance(ctrl1, accx, accy, render, sound);
}

// advance a frame and possibly subframe reset
GBHawk_EXPORT bool GB_subframe_advance(GBCore* p, uint8_t ctrl1, uint16_t accx, uint16_t accy, bool render, bool sound, bool do_reset, uint32_t reset_cycle)
{
	return p->SubFrameAdvance(ctrl1, accx, accy, render, sound, do_reset, reset_cycle);
}

// send video data to external video provider
GBHawk_EXPORT void GB_get_video(GBCore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
GBHawk_EXPORT uint32_t GB_get_audio(GBCore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R);
}

GBHawk_EXPORT void GB_setrumblecallback(GBCore* p, void (*callback)(bool))
{
	p->SetRumbleCallback(callback);
}

#pragma region State Save / Load

// save state
GBHawk_EXPORT void GB_save_state(GBCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBHawk_EXPORT void GB_load_state(GBCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBHawk_EXPORT uint8_t GB_getsysbus(GBCore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

GBHawk_EXPORT uint8_t GB_getvram(GBCore* p, uint32_t addr) {
	return p->GetVRAM(addr);
}

GBHawk_EXPORT uint8_t GB_getwram(GBCore* p, uint32_t addr) {
	return p->GetWRAM(addr);
}

GBHawk_EXPORT uint8_t GB_getiwram(GBCore* p, uint32_t addr) {
	return p->GetIWRAM(addr);
}

GBHawk_EXPORT uint8_t GB_getoam(GBCore* p, uint32_t addr) {
	return p->GetOAM(addr);
}

GBHawk_EXPORT uint8_t GB_getpalram(GBCore* p, uint32_t addr) {
	return p->GetPALRAM(addr);
}

GBHawk_EXPORT uint8_t GB_getregisters(GBCore* p, uint32_t addr) {
	return p->GetRegisters(addr);
}

GBHawk_EXPORT uint8_t GB_getsram(GBCore* p, uint32_t addr) {
	return p->GetSRAM(addr);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBHawk_EXPORT void GB_settracecallback(GBCore* p, void (*callback)(int)) {
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
GBHawk_EXPORT int GB_getheaderlength(GBCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBHawk_EXPORT int GB_getdisasmlength(GBCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBHawk_EXPORT int GB_getregstringlength(GBCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBHawk_EXPORT void GB_getheader(GBCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBHawk_EXPORT void GB_getregisterstate(GBCore* p, char* r, int t, int l) {
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
GBHawk_EXPORT void GB_getdisassembly(GBCore* p, char* d, int t, int l) {
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
GBHawk_EXPORT void GB_setmessagecallback(GBCore* p, void (*callback)(int)) {
	p->SetMessageCallback(callback);
}

// get message
GBHawk_EXPORT void GB_getmessage(GBCore* p, char* d) {
	p->GetMessage(d);
}

#pragma endregion

#pragma region PPU Viewer

// set tracer callback
GBHawk_EXPORT uint8_t* GB_get_ppu_pntrs(GBCore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set scanline callback
GBHawk_EXPORT void GB_setscanlinecallback(GBCore* p, void (*callback)(void), int sl) {
	p->SetScanlineCallback(callback, sl);
}

#pragma endregion

#pragma endregion

#pragma region Link Core
// Create pointer to a core instance
GBHawk_EXPORT GBLinkCore* GBLink_create()
{
	return new GBLinkCore();
}

// free the memory from the core pointer
GBHawk_EXPORT void GBLink_destroy(GBLinkCore* p)
{
	delete p->L.GB.Cart_RAM;
	delete p->R.GB.Cart_RAM;

	std::free(p);
}

// load bios and basic into the core
GBHawk_EXPORT void GBLink_load_bios(GBLinkCore* p, uint8_t* bios)
{
	p->Load_BIOS(bios);
}

// load a rom into the core
GBHawk_EXPORT void GBLink_load(GBLinkCore* p, uint8_t* rom_0, uint32_t size_0, uint32_t mapper_0,
												 uint8_t* rom_1, uint32_t size_1, uint32_t mapper_1,
												 uint64_t datetime_0, bool rtc_functional_0,
												 uint64_t datetime_1, bool rtc_functional_1,
												 int16_t EEPROM_offset_0, int16_t EEPROM_offset_1,
												 uint16_t flash_type_64_value_0, uint16_t flash_type_64_value_1,
												 uint16_t flash_type_128_value_0, uint16_t flash_type_128_value_1,
												 int16_t flash_write_offset_0, int16_t flash_write_offset_1,
												 int32_t flash_sector_offset_0, int32_t flash_sector_offset_1,
												 int32_t flash_chip_offset_0, int32_t flash_chip_offset_1,
												 bool is_GBP_0, bool is_GBP_1)
{
	p->Load_ROM(rom_0, size_0, mapper_0, rom_1, size_1, mapper_1, datetime_0, rtc_functional_0, datetime_1, rtc_functional_1,
				EEPROM_offset_0, EEPROM_offset_1, flash_type_64_value_0, flash_type_64_value_1, flash_type_128_value_0, flash_type_128_value_1,
			    flash_write_offset_0, flash_write_offset_1, flash_sector_offset_0, flash_sector_offset_1, flash_chip_offset_0, flash_chip_offset_1, is_GBP_0, is_GBP_1);
}

// Create a default SRAM
GBHawk_EXPORT void GBLink_create_SRAM(GBLinkCore* p, uint8_t* sram, uint32_t size, uint32_t num)
{
	p->Create_SRAM(sram, size, num);
}

// load sram into the core
GBHawk_EXPORT void GBLink_load_SRAM(GBLinkCore* p, uint8_t* sram, uint32_t size, uint32_t num)
{
	p->Load_SRAM(sram, size, num);
}

// reset the system
GBHawk_EXPORT void GBLink_Hard_Reset(GBLinkCore* p)
{
	p->Hard_Reset();
}

// enable GBP
GBHawk_EXPORT void GBLink_Set_GBP_Enable(GBLinkCore* p, uint32_t num)
{
	p->Set_GBP_Enable(num);
}

// advance a frame
GBHawk_EXPORT bool GBLink_frame_advance(GBLinkCore* p, uint16_t ctrl_0, uint16_t accx_0, uint16_t accy_0, uint8_t solar_0, bool render_0, bool sound_0,
														  uint16_t ctrl_1, uint16_t accx_1, uint16_t accy_1, uint8_t solar_1, bool render_1, bool sound_1,
														  bool l_reset, bool r_reset)
{
	return p->FrameAdvance(ctrl_0, accx_0, accy_0, solar_0, render_0, sound_0, ctrl_1, accx_1, accy_1, solar_1, render_1, sound_1, l_reset, r_reset);
}

// send video data to external video provider
GBHawk_EXPORT void GBLink_get_video(GBLinkCore* p, uint32_t* dest, uint32_t num)
{
	p->GetVideo(dest, num);
}

// send audio data to external audio provider
GBHawk_EXPORT uint32_t GBLink_get_audio(GBLinkCore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R, uint32_t num)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R, num);
}

#pragma region State Save / Load

// save state
GBHawk_EXPORT void GBLink_save_state(GBLinkCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBHawk_EXPORT void GBLink_load_state(GBLinkCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

// set rumble callback
GBHawk_EXPORT void GBLink_setrumblecallback(GBLinkCore* p, void (*callback)(bool), uint32_t num) {
	p->SetRumbleCallback(callback, num);
}

#pragma endregion

#pragma region Memory Domain Functions

GBHawk_EXPORT uint8_t GBLink_getsysbus(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetSysBus(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getvram(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetVRAM(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getwram(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetWRAM(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getiwram(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetIWRAM(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getoam(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetOAM(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getpalram(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetPALRAM(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getsram(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetSRAM(addr, num);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBHawk_EXPORT void GBLink_settracecallback(GBLinkCore* p, void (*callback)(int), uint32_t num) {
	p->SetTraceCallback(callback, num);
}

// return the cpu trace header length
GBHawk_EXPORT int GBLink_getheaderlength(GBLinkCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBHawk_EXPORT int GBLink_getdisasmlength(GBLinkCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBHawk_EXPORT int GBLink_getregstringlength(GBLinkCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBHawk_EXPORT void GBLink_getheader(GBLinkCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBHawk_EXPORT void GBLink_getregisterstate(GBLinkCore* p, char* r, int t, int l, uint32_t num) {
	p->GetRegisterState(r, t, l, num);
}

// return the cpu disassembly
GBHawk_EXPORT void GBLink_getdisassembly(GBLinkCore* p, char* d, int t, int l, uint32_t num) {
	p->GetDisassembly(d, t, l, num);
}

#pragma endregion

#pragma endregion