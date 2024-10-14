// GBAHawk.cpp : Defines the exported functions for the DLL.

#include "GBAHawk.h"
#include "Core.h"
#include "LinkCore.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace GBAHawk;

#pragma region Core
// Create pointer to a core instance
GBAHawk_EXPORT GBACore* GBA_create()
{
	return new GBACore();
}

// free the memory from the core pointer
GBAHawk_EXPORT void GBA_destroy(GBACore* p)
{
	delete p->GBA.Cart_RAM;

	std::free(p);
}

// load bios and basic into the core
GBAHawk_EXPORT void GBA_load_bios(GBACore* p, uint8_t* bios)
{
	p->Load_BIOS(bios);
}

// load a rom into the core
GBAHawk_EXPORT void GBA_load(GBACore* p, uint8_t* rom, uint32_t size, uint32_t mapper, uint64_t datetime, bool rtc_functional, int16_t EEPROM_offset, uint16_t flash_type_64_value,
							uint16_t flash_type_128_value, int16_t flash_write_offset, int32_t flash_sector_offset, int32_t flash_chip_offset, bool is_GBP)
{
	p->Load_ROM(rom, size, mapper, datetime, rtc_functional, EEPROM_offset, flash_type_64_value, flash_type_128_value, flash_write_offset, flash_sector_offset, flash_chip_offset, is_GBP);
}

// Create a default SRAM
GBAHawk_EXPORT void GBA_create_SRAM(GBACore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
GBAHawk_EXPORT void GBA_load_SRAM(GBACore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

// reset the system
GBAHawk_EXPORT void GBA_Hard_Reset(GBACore* p)
{
	p->Hard_Reset();
}

// enable GBP
GBAHawk_EXPORT void GBA_Set_GBP_Enable(GBACore* p)
{
	p->Set_GBP_Enable();
}

// advance a frame
GBAHawk_EXPORT bool GBA_frame_advance(GBACore* p, uint16_t ctrl1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool sound)
{
	return p->FrameAdvance(ctrl1, accx, accy, solar, render, sound);
}

// advance a frame and possibly subframe reset
GBAHawk_EXPORT bool GBA_subframe_advance(GBACore* p, uint16_t ctrl1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool sound, bool do_reset, uint32_t reset_cycle)
{
	return p->SubFrameAdvance(ctrl1, accx, accy, solar, render, sound, do_reset, reset_cycle);
}

// send video data to external video provider
GBAHawk_EXPORT void GBA_get_video(GBACore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
GBAHawk_EXPORT uint32_t GBA_get_audio(GBACore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R);
}

GBAHawk_EXPORT void GBA_setrumblecallback(GBACore* p, void (*callback)(bool))
{
	p->SetRumbleCallback(callback);
}

#pragma region State Save / Load

// save state
GBAHawk_EXPORT void GBA_save_state(GBACore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBAHawk_EXPORT void GBA_load_state(GBACore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBAHawk_EXPORT uint8_t GBA_getsysbus(GBACore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

GBAHawk_EXPORT uint8_t GBA_getvram(GBACore* p, uint32_t addr) {
	return p->GetVRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getwram(GBACore* p, uint32_t addr) {
	return p->GetWRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getiwram(GBACore* p, uint32_t addr) {
	return p->GetIWRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getoam(GBACore* p, uint32_t addr) {
	return p->GetOAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getpalram(GBACore* p, uint32_t addr) {
	return p->GetPALRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getsram(GBACore* p, uint32_t addr) {
	return p->GetSRAM(addr);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBAHawk_EXPORT void GBA_settracecallback(GBACore* p, void (*callback)(int)) {
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
GBAHawk_EXPORT int GBA_getheaderlength(GBACore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBAHawk_EXPORT int GBA_getdisasmlength(GBACore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBAHawk_EXPORT int GBA_getregstringlength(GBACore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBAHawk_EXPORT void GBA_getheader(GBACore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBAHawk_EXPORT void GBA_getregisterstate(GBACore* p, char* r, int t, int l) {
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
GBAHawk_EXPORT void GBA_getdisassembly(GBACore* p, char* d, int t, int l) {
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
GBAHawk_EXPORT void GBA_setmessagecallback(GBACore* p, void (*callback)(int)) {
	p->SetMessageCallback(callback);
}

// get message
GBAHawk_EXPORT void GBA_getmessage(GBACore* p, char* d, int l) {
	p->GetMessage(d, l);
}

#pragma endregion

#pragma region PPU Viewer

// set tracer callback
GBAHawk_EXPORT uint8_t* GBA_get_ppu_pntrs(GBACore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set scanline callback
GBAHawk_EXPORT void GBA_setscanlinecallback(GBACore* p, void (*callback)(void), int sl) {
	p->SetScanlineCallback(callback, sl);
}

#pragma endregion

#pragma endregion

#pragma region Link Core
// Create pointer to a core instance
GBAHawk_EXPORT GBALinkCore* GBALink_create()
{
	return new GBALinkCore();
}

// free the memory from the core pointer
GBAHawk_EXPORT void GBALink_destroy(GBALinkCore* p)
{
	delete p->L.GBA.Cart_RAM;
	delete p->R.GBA.Cart_RAM;

	std::free(p);
}

// load bios and basic into the core
GBAHawk_EXPORT void GBALink_load_bios(GBALinkCore* p, uint8_t* bios)
{
	p->Load_BIOS(bios);
}

// load a rom into the core
GBAHawk_EXPORT void GBALink_load(GBALinkCore* p, uint8_t* rom_0, uint32_t size_0, uint32_t mapper_0,
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
GBAHawk_EXPORT void GBALink_create_SRAM(GBALinkCore* p, uint8_t* sram, uint32_t size, uint32_t num)
{
	p->Create_SRAM(sram, size, num);
}

// load sram into the core
GBAHawk_EXPORT void GBALink_load_SRAM(GBALinkCore* p, uint8_t* sram, uint32_t size, uint32_t num)
{
	p->Load_SRAM(sram, size, num);
}

// reset the system
GBAHawk_EXPORT void GBALink_Hard_Reset(GBALinkCore* p)
{
	p->Hard_Reset();
}

// enable GBP
GBAHawk_EXPORT void GBALink_Set_GBP_Enable(GBALinkCore* p, uint32_t num)
{
	p->Set_GBP_Enable(num);
}

// advance a frame
GBAHawk_EXPORT bool GBALink_frame_advance(GBALinkCore* p, uint16_t ctrl_0, uint16_t accx_0, uint16_t accy_0, uint8_t solar_0, bool render_0, bool sound_0,
														  uint16_t ctrl_1, uint16_t accx_1, uint16_t accy_1, uint8_t solar_1, bool render_1, bool sound_1,
														  bool l_reset, bool r_reset)
{
	return p->FrameAdvance(ctrl_0, accx_0, accy_0, solar_0, render_0, sound_0, ctrl_1, accx_1, accy_1, solar_1, render_1, sound_1, l_reset, r_reset);
}

// send video data to external video provider
GBAHawk_EXPORT void GBALink_get_video(GBALinkCore* p, uint32_t* dest, uint32_t num)
{
	p->GetVideo(dest, num);
}

// send audio data to external audio provider
GBAHawk_EXPORT uint32_t GBALink_get_audio(GBALinkCore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R, uint32_t num)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R, num);
}

#pragma region State Save / Load

// save state
GBAHawk_EXPORT void GBALink_save_state(GBALinkCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBAHawk_EXPORT void GBALink_load_state(GBALinkCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

// set rumble callback
GBAHawk_EXPORT void GBALink_setrumblecallback(GBALinkCore* p, void (*callback)(bool), uint32_t num) {
	p->SetRumbleCallback(callback, num);
}

#pragma endregion

#pragma region Memory Domain Functions

GBAHawk_EXPORT uint8_t GBALink_getsysbus(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetSysBus(addr, num);
}

GBAHawk_EXPORT uint8_t GBALink_getvram(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetVRAM(addr, num);
}

GBAHawk_EXPORT uint8_t GBALink_getwram(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetWRAM(addr, num);
}

GBAHawk_EXPORT uint8_t GBALink_getiwram(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetIWRAM(addr, num);
}

GBAHawk_EXPORT uint8_t GBALink_getoam(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetOAM(addr, num);
}

GBAHawk_EXPORT uint8_t GBALink_getpalram(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetPALRAM(addr, num);
}

GBAHawk_EXPORT uint8_t GBALink_getsram(GBALinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetSRAM(addr, num);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBAHawk_EXPORT void GBALink_settracecallback(GBALinkCore* p, void (*callback)(int), uint32_t num) {
	p->SetTraceCallback(callback, num);
}

// return the cpu trace header length
GBAHawk_EXPORT int GBALink_getheaderlength(GBALinkCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBAHawk_EXPORT int GBALink_getdisasmlength(GBALinkCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBAHawk_EXPORT int GBALink_getregstringlength(GBALinkCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBAHawk_EXPORT void GBALink_getheader(GBALinkCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBAHawk_EXPORT void GBALink_getregisterstate(GBALinkCore* p, char* r, int t, int l, uint32_t num) {
	p->GetRegisterState(r, t, l, num);
}

// return the cpu disassembly
GBAHawk_EXPORT void GBALink_getdisassembly(GBALinkCore* p, char* d, int t, int l, uint32_t num) {
	p->GetDisassembly(d, t, l, num);
}

#pragma endregion

#pragma endregion