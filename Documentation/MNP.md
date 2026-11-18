# MNP/V.42 Protocol

The Newton uses
[MNP/V.42 Annex A](https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-V.42-200203-I!!PDF-E&type=items)
framing for flow control and error correction over serial links. DCL implements
a simplified version using a state machine. The simplifications are:

- Any protocol error such as timeouts for expected frames is treated as a fatal error,
  the implementation does not retransmit and expects an error free serial link.
- DCL advertises a window size of two outstanding link transmission frames,
  but will only ever use one to simplify frame tracking. The reason to
  advertise a window of two is to trigger the keep-alive sequence over
  inspector links. The Newton will in this mode repeatedly send link
  acknowledgement frames, and expects the same frame type as an answer. The Newton
  will not enter this mode unless the window size is larger than one.