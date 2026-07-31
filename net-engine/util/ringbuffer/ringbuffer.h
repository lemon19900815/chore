#ifndef __RING_BUFFER_INC_H__
#define __RING_BUFFER_INC_H__

#include <string>
#include <string.h>

#if defined(_MSC_VER)
#pragma warning(disable: 4389)
#endif

// Linux : kfifo interface

#ifdef _WIN32
#include <Windows.h>
#include <Winnt.h>
#define mb() MemoryBarrier()
#define rmb() MemoryBarrier()
#define wmb() MemoryBarrier()
#elif defined __LINUX__
  // Memory barrier, copy from linux kernel
#define __ASM_FORM(x)	" " #x " "
#define __ASM_SEL(a,b) __ASM_FORM(a)

#define _ASM_ALIGN	__ASM_SEL(.balign 4, .balign 8)
#define _ASM_PTR	__ASM_SEL(.long, .quad)
#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)

#define ALTERNATIVE(oldinstr, newinstr, feature)			\
  \
  "661:\n\t" oldinstr "\n662:\n"					\
  ".section .altinstructions,\"a\"\n"				\
  _ASM_ALIGN "\n"							\
  _ASM_PTR "661b\n"				/* label           */	\
  _ASM_PTR "663f\n"				/* new instruction */	\
  "	 .byte " __stringify(feature) "\n"	/* feature bit     */	\
  "	 .byte 662b-661b\n"			/* sourcelen       */	\
  "	 .byte 664f-663f\n"			/* replacementlen  */	\
  ".previous\n"							\
  ".section .altinstr_replacement, \"ax\"\n"			\
  "663:\n\t" newinstr "\n664:\n"		/* replacement     */	\
  ".previous"

#define alternative(oldinstr, newinstr, feature) \
  asm volatile (ALTERNATIVE(oldinstr, newinstr, feature) : : : "memory")

#define X86_FEATURE_XMM		(0*32+25) /* "sse" */
#define X86_FEATURE_XMM2	(0*32+26) /* "sse2" */

  /*
  * Force strict CPU ordering.
  * And yes, this is required on UP too when we're talking
  * to devices.
  */

  //#if defined CONFIG_X86_32
  /*
  * Some non-Intel clones support out of order store. wmb() ceases to be a
  * nop for these.
  */
  //#define mb() alternative("lock; addl $0,0(%%esp)", "mfence", X86_FEATURE_XMM2)
  //#define rmb() alternative("lock; addl $0,0(%%esp)", "lfence", X86_FEATURE_XMM2)
  //#define wmb() alternative("lock; addl $0,0(%%esp)", "sfence", X86_FEATURE_XMM)
  //#else
#define mb() asm volatile("mfence":::"memory")
#define rmb() asm volatile("lfence":::"memory")
#define wmb() asm volatile("sfence" ::: "memory")
  //#endif // CONFIG_X86_32

#else
#error "Herm don't support this platform!"
#endif // #if defined __linux__

#define smp_mb() mb()
#define smp_rmb() rmb()
#define smp_wmb() wmb()

// 无锁缓冲队列.
class RingBuffer final
{
public:
  explicit RingBuffer(int buf_size);

  ~RingBuffer();

  inline void clear()
  {
    m_write_p = 0;
    m_read_p = 0;
  }

  inline unsigned int available() const
  {
    return m_buffer_size - (m_write_p - m_read_p);
  }

  inline unsigned int used() const
  {
    return m_write_p - m_read_p;
  }

  inline bool empty() const
  {
    return used() == 0;
  }

  inline bool full() const
  {
    auto tmp = used();
    return m_buffer_size == static_cast<int>(tmp);
  }

  /*
  * Read data but don't move the m_read_p.
  */
  unsigned int __peek(char* buffer, unsigned int len);
  /*
  * Move the m_read_p
  */
  bool __seekread(unsigned int len);

  /*
  * put_data - puts some data into the FIFO, no locking version
  * Note that with only one concurrent reader and one concurrent
  * writer, you don't need extra locking to use these functions.
  */
  unsigned int __kfifo_put(const char* buffer, unsigned int len);
  unsigned int __kfifo_put(const std::string& buffer, unsigned int len);

  /*
  * get_data - gets some data from the FIFO, no locking version
  * Note that with only one concurrent reader and one concurrent
  * writer, you don't need extra locking to use these functions.
  */
  unsigned int __kfifo_get(char* buffer, unsigned int len);
  unsigned int __kfifo_get(std::string& buffer, unsigned int len);

protected:
  inline unsigned int _max(unsigned int a, unsigned int b)
  {
    return a > b ? a : b;
  }

  inline unsigned int _min(unsigned int a, unsigned int b)
  {
    return a < b ? a : b;
  }

  // 2^n
  inline bool is_power_of_2(unsigned int x) const
  {
    return (x != 0) && ((x & (x - 1)) == 0);
  }

private:
  int m_buffer_size;
  char* m_circle_buffer;
  unsigned int m_write_p;
  unsigned int m_read_p;
};
#endif // !__RING_BUFFER_INC_H__
