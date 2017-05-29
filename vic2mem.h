#ifndef VIC2MEM_H
#define VIC2MEM_H

#include "tedmem.h"

class KEYS64;
struct Color;

#define FAST_BOOT 1
#define VIC_PIXELS_PER_ROW 504
#define VIC_REAL_CLOCK_M10 9852480 // 9852480 19704960
#define VIC_SOUND_CLOCK (312*63*50)
#define VIC_REAL_SOUND_CLOCK (VIC_REAL_CLOCK_M10 / 8 / 10)

class Vic2mem : public TED
{
    public:
        Vic2mem();
        virtual ~Vic2mem();
		virtual KEYS *getKeys() { return (KEYS*) keys64; }
		virtual void loadroms();
        virtual void Reset(bool clearmem);
        virtual void soundReset();
        // read memory through memory decoder
        virtual unsigned char Read(unsigned int addr);
        virtual void Write(unsigned int addr, unsigned char value);
		virtual void poke(unsigned int addr, unsigned char data) { Ram[addr & 0xffff] = data; }
        virtual void ted_process(const unsigned int continuous);
		virtual void setCpuPtr(CPU *cpu);
		//virtual unsigned int getColorCount() { return 256; };
		virtual Color getColor(unsigned int ix);
		virtual unsigned int getCyclesPerRow() const { return 504; }
		void latchCounters();
		virtual void copyToKbBuffer(const char *text, unsigned int length = 0);
		virtual unsigned int getSoundClock() { return VIC_SOUND_CLOCK; }
		virtual unsigned int getRealSlowClock() { return VIC_REAL_CLOCK_M10 / 10; }
		virtual unsigned int getEmulationLevel() { return 2; }
#if !FAST_BOOT
		virtual unsigned int getAutostartDelay() { return 175; }
#else
		virtual unsigned int getAutostartDelay() { return 50; }
#endif
		virtual unsigned short getEndLoadAddressPtr() { return 0xAE; };
		virtual unsigned int getHorizontalCount() { return beamx; }
		// this is for the savestate support
		virtual void dumpState();
		virtual void readState();

    protected:
		void doHRetrace();
		void newLine();
		void changeCharsetBank();
		void checkIRQflag();
		void doDelayedDMA();
		void UpdateSerialState(unsigned char newPort);
		unsigned char vicReg[0x40];
		//
		struct Mob {
			bool enabled;
			bool multicolor;
			bool priority;
			unsigned int expandX;
			unsigned int expandY;
			unsigned int y;
			unsigned int x;
			unsigned char color;
			unsigned int dataCount;
			unsigned int dataCountReload;
			unsigned int dataAddress;
			unsigned int reloadFlipFlop;
			bool dmaState;
			bool rendering;
			bool finished;
			union SpriteDma {
				unsigned char shiftRegBuf[4];
				unsigned int dwSrDmaBuf;
			} sdb[2];
		} mob[8];
		unsigned char spriteCollisionReg;
		unsigned char spriteBckgCollReg;
		unsigned char spriteCollisions[512 + 48];
		unsigned char spriteBckgColl[512 + 48];
		unsigned char collisionLookup[256];
		unsigned char mobExtCol[4];
		void renderSprite(unsigned char *in, unsigned char *out, Mob &m, unsigned int cx, const unsigned int six);
		void drawSpritesPerLine(unsigned char *lineBuf);
		bool checkSpriteDMA(unsigned int i);
		//
		struct CIA {
			CIA() { refCount++; irqCallback = 0;}
			~CIA() { refCount--; }
			unsigned char pra;
			unsigned char prb;
			unsigned char prbTimerToggle;
			unsigned char prbTimerOut;
			unsigned char ddra;
			unsigned char ddrb;
			int ta;
			int tb;
			int latcha;
			int latchb;
			int taFeed;
			int tbFeed;
			unsigned char cra;
			unsigned char crb;
			unsigned char sdr;
			unsigned int sdrShiftCnt;
			unsigned char icr;
			unsigned char irq_mask;
			struct TOD {
                unsigned int sec;
                unsigned int min;
                unsigned int tenths;
                unsigned int halt;
                unsigned int hr;
				unsigned int ampm;
                bool latched;
			} alm, tod, todLatch;
			unsigned int todIn;
			unsigned char reg[16];
			void reset();
			void write(unsigned int addr, unsigned char value);
			unsigned char read(unsigned int addr);
			void setIRQflag(unsigned int mask);
			void countTimers();
			void countTimerB(bool cascaded);
			void todUpdate();
			static unsigned int bcd2hex(unsigned int bcd);
			static unsigned int hex2bcd(unsigned int hex);
			static unsigned int tod2frames(TOD &todin);
			static void frames2tod(unsigned int frames, TOD &todout, unsigned int frq);
			unsigned int todCount, alarmCount;
			static unsigned int refCount;
			CallBackFunctor irqCallback;
			void *callBackParam;
			void setIrqCallback(CallBackFunctor irqCallback_, void *param) {
				irqCallback = irqCallback_;
				callBackParam = param;
			}
			bool pendingIrq;
		} cia[2];
		static void setCiaIrq(void *param);
		static void setCiaNmi(void *param);
		unsigned char *vicBase;
		void	hi_text();
		void	mc_text();
		void	ec_text();
		void	mcec();
		void	hi_bmec();
		void	mc_bmec();
		void	hi_bitmap();
		void	mc_bitmap();
		void render();
		unsigned char *colorRAM;
		KEYS64 *keys64;
    private:
		unsigned char portState;
		unsigned int dmaCount;
		ClockCycle vicBusAccessCycleStart;
		unsigned int spriteDMAmask;
		void doXscrollChange(unsigned int oldXscr, unsigned int newXscr);
		unsigned char readFloatingBus(unsigned int adr);
		void checkSpriteEnable();
		void stopSpriteDMA();
		void spriteReloadCounters();
		unsigned int lpLatchX, lpLatchY;
		bool lpLatched;
};

#endif // VIC2MEM_H
