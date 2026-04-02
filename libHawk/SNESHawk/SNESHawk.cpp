// SNESHawk.cpp : Defines the exported functions for the DLL.

#include "SNESHawk.h"
#include "Core.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace SNESHawk;

#pragma region Core
// Create pointer to a core instance
SNESHawk_EXPORT SNESCore* SNES_create()
{
	return new SNESCore();
}

// free the memory from the core pointer
SNESHawk_EXPORT void SNES_destroy(SNESCore* p)
{
	delete p->SNES.Cart_RAM;

	std::free(p);
}

// load a rom into the core
SNESHawk_EXPORT void SNES_load(SNESCore* p, uint8_t* rom, uint32_t size, uint8_t* header, bool mmc3_old_irq, bool mapper_bus_conflicts, bool apu_test_regs, bool cpu_zero_reset)
{
	p->Load_ROM(rom, size, header, mmc3_old_irq, mapper_bus_conflicts, apu_test_regs, cpu_zero_reset);
}

// Create a default SRAM
SNESHawk_EXPORT void SNES_create_SRAM(SNESCore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
SNESHawk_EXPORT void SNES_load_SRAM(SNESCore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

// load palette into the core
SNESHawk_EXPORT void SNES_load_Palette(SNESCore* p, uint8_t* palette)
{
	p->Load_Palette(palette);
}

// reset the system
SNESHawk_EXPORT void SNES_Hard_Reset(SNESCore* p)
{
	p->Hard_Reset();
}

// soft reset the system
SNESHawk_EXPORT void SNES_Soft_Reset(SNESCore* p)
{
	p->Soft_Reset();
}

// advance a frame
SNESHawk_EXPORT bool SNES_frame_advance(SNESCore* p, bool render, bool sound)
{
	return p->FrameAdvance( render, sound);
}

// get cpu clocks
SNESHawk_EXPORT uint64_t SNES_get_cycles(SNESCore* p)
{
	return p->SNES.TotalExecutedCycles;
}

// advance a frame and possibly subframe reset
SNESHawk_EXPORT bool SNES_subframe_advance(SNESCore* p, bool render, bool sound, bool do_reset, uint32_t reset_cycle)
{
	return p->SubFrameAdvance(render, sound, do_reset, reset_cycle);
}

// get clocks needed for timing subframe movies
SNESHawk_EXPORT uint64_t SNES_subframe_cycles(SNESCore* p)
{
	return p->SNES.Total_CPU_Clock_Cycles;
}

// set controller read callback
SNESHawk_EXPORT void SNES_setcontrollercallback(SNESCore* p, uint8_t (*callback)(bool)) {
	p->SetControllerCallback(callback);
}

// set controller strobe callback
SNESHawk_EXPORT void SNES_setstrobecallback(SNESCore* p, void(*callback)(uint8_t, uint8_t)) {
	p->SetStrobeCallback(callback);
}

// send video data to external video provider
SNESHawk_EXPORT void SNES_get_video(SNESCore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
SNESHawk_EXPORT uint32_t SNES_get_audio(SNESCore* p, int32_t* dest, int32_t* n_samp)
{
	return p->GetAudio(dest, n_samp);
}

#pragma region State Save / Load

// save state
SNESHawk_EXPORT void SNES_save_state(SNESCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
SNESHawk_EXPORT void SNES_load_state(SNESCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

SNESHawk_EXPORT uint8_t SNES_getsysbus(SNESCore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

SNESHawk_EXPORT uint8_t SNES_getapubus(SNESCore* p, uint32_t addr) {
	return p->GetAPUBus(addr);
}

SNESHawk_EXPORT uint8_t SNES_getvram(SNESCore* p, uint32_t addr) {
	return p->GetVRAM(addr);
}

SNESHawk_EXPORT uint8_t SNES_getchrrom(SNESCore* p, uint32_t addr) {
	return p->GetCHR_ROM(addr);
}

SNESHawk_EXPORT uint8_t SNES_getram(SNESCore* p, uint32_t addr) {
	return p->GetRAM(addr);
}

SNESHawk_EXPORT uint8_t SNES_getrom(SNESCore* p, uint32_t addr) {
	return p->GetROM(addr);
}

SNESHawk_EXPORT uint8_t SNES_getoam(SNESCore* p, uint32_t addr) {
	return p->GetOAM(addr);
}

SNESHawk_EXPORT uint8_t SNES_getpalram(SNESCore* p, uint32_t addr) {
	return p->GetPALRAM(addr);
}

SNESHawk_EXPORT uint8_t SNES_getsram(SNESCore* p, uint32_t addr) {
	return p->GetSRAM(addr);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
SNESHawk_EXPORT void SNES_settracecallback(SNESCore* p, void (*callback)(int)) {
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
SNESHawk_EXPORT int SNES_getheaderlength(SNESCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
SNESHawk_EXPORT int SNES_getdisasmlength(SNESCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
SNESHawk_EXPORT int SNES_getregstringlength(SNESCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
SNESHawk_EXPORT void SNES_getheader(SNESCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
SNESHawk_EXPORT void SNES_getregisterstate(SNESCore* p, char* r, int t, int l) {
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
SNESHawk_EXPORT void SNES_getdisassembly(SNESCore* p, char* d, int t, int l) {
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
SNESHawk_EXPORT void SNES_setmessagecallback(SNESCore* p, void (*callback)(int)) {
	p->SetMessageCallback(callback);
}

// get message
SNESHawk_EXPORT void SNES_getmessage(SNESCore* p, char* d) {
	p->GetMessage(d);
}

// setinput callback
SNESHawk_EXPORT void SNES_setinputpollcallback(SNESCore* p, void (*callback)()) {
	p->SetInputPollCallback(callback);
}


#pragma endregion

#pragma region PPU Viewer

// get values from ppu

SNESHawk_EXPORT bool SNES_get_ppu_vals(SNESCore* p, uint32_t sel)
{
	return p->Get_PPU_Values(sel);
}

// get values from ppu bus
SNESHawk_EXPORT uint8_t SNES_get_ppu_bus(SNESCore* p, uint32_t addr)
{
	return p->Get_PPU_Bus_Peek(addr);
}

// get values from ppu bus as seen by board
SNESHawk_EXPORT uint8_t SNES_get_board_peek_ppu(SNESCore* p, uint32_t addr)
{
	return p->Get_PPU_Board_Peek(addr);
}

// get pointers from ppu
SNESHawk_EXPORT uint8_t* SNES_get_ppu_pntrs(SNESCore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set nametable viewer callback
SNESHawk_EXPORT void SNES_setntvcallback(SNESCore* p, void (*callback)(void), int sl) {
	p->SetNTVCallback(callback, sl);
}

// set ppu viewer callback
SNESHawk_EXPORT void SNES_setppucallback(SNESCore* p, void (*callback)(void), int sl) {
	p->SetPPUCallback(callback, sl);
}

#pragma endregion

#pragma endregion