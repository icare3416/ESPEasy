// Copyright 2017 David Conran

#include "IRrecv_test.h"
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for the IRrecv object.
TEST(TestIRrecv, DefaultBufferSize) {
  IRrecv irrecv_default(1);
  EXPECT_EQ(kRawBuf, irrecv_default.getBufSize());
}

TEST(TestIRrecv, LargeBufferSize) {
  IRrecv irrecv_large(3, 1024);
  EXPECT_EQ(1024, irrecv_large.getBufSize());
}

TEST(TestIRrecv, SmallBufferSize) {
  IRrecv irrecv_small(4, 80);
  EXPECT_EQ(80, irrecv_small.getBufSize());
}

TEST(TestIRrecv, MediumBufferSize) {
  IRrecv irrecv_medium(4, 512);
  EXPECT_EQ(512, irrecv_medium.getBufSize());
}

TEST(TestIRrecv, IRrecvDestructor) {
  IRrecv *irrecv_ptr = new IRrecv(1);
  EXPECT_EQ(kRawBuf, irrecv_ptr->getBufSize());

  delete irrecv_ptr;
  irrecv_ptr = new IRrecv(1, 1234);
  EXPECT_EQ(1234, irrecv_ptr->getBufSize());
  delete irrecv_ptr;

  irrecv_ptr = new IRrecv(1, 123);
  EXPECT_EQ(123, irrecv_ptr->getBufSize());
  delete irrecv_ptr;
}

// Tests for copyIrParams()

TEST(TestCopyIrParams, CopyEmpty) {
  irparams_t src;
  irparams_t dst;
  uint16_t test_size = 1234;
  src.bufsize = test_size;
  src.rawlen = 0;
  src.rawbuf = new uint16_t[test_size];
  src.overflow = false;
  dst.bufsize = 4567;
  dst.rawlen = 123;
  dst.rawbuf = new uint16_t[test_size];
  dst.overflow = true;
  // Confirm we are looking at different memory for the buffers.
  ASSERT_NE(src.rawbuf, dst.rawbuf);

  IRrecv irrecv(4);
  irrecv.copyIrParams(&src, &dst);

  ASSERT_EQ(src.bufsize, dst.bufsize);
  ASSERT_EQ(src.rawlen, dst.rawlen);
  ASSERT_NE(src.rawbuf, dst.rawbuf);  // Pointers, not content.
  ASSERT_EQ(src.overflow, dst.overflow);
  // Contents of the buffers needs to match.
  EXPECT_EQ(0, memcmp(src.rawbuf, dst.rawbuf, src.bufsize * sizeof(uint16_t)));
}

TEST(TestCopyIrParams, CopyNonEmpty) {
  irparams_t src;
  irparams_t dst;
  uint16_t test_size = 1234;
  src.bufsize = test_size;
  src.rawlen = 67;
  src.rawbuf = new uint16_t[test_size];
  src.rawbuf[0] = 0xF00D;
  src.rawbuf[1] = 0xBEEF;
  src.rawbuf[test_size - 1] = 0xDEAD;
  src.overflow = true;
  dst.bufsize = 0;
  dst.rawlen = 0;
  dst.rawbuf = new uint16_t[test_size];
  dst.overflow = false;
  // Confirm we are looking at different memory for the buffers.
  ASSERT_NE(src.rawbuf, dst.rawbuf);
  // and that they differ before we test.
  EXPECT_NE(0, memcmp(src.rawbuf, dst.rawbuf, src.bufsize * sizeof(uint16_t)));

  IRrecv irrecv(4);
  irrecv.copyIrParams(&src, &dst);

  ASSERT_EQ(src.bufsize, dst.bufsize);
  EXPECT_EQ(test_size, dst.bufsize);
  ASSERT_EQ(src.rawlen, dst.rawlen);
  EXPECT_EQ(67, dst.rawlen);
  ASSERT_EQ(src.overflow, dst.overflow);
  EXPECT_TRUE(dst.overflow);
  ASSERT_NE(src.rawbuf, dst.rawbuf);  // Pointers, not content.
  // Contents of the buffers needs to match.
  EXPECT_EQ(0, memcmp(src.rawbuf, dst.rawbuf, src.bufsize * sizeof(uint16_t)));
  // Check the canary values.
  EXPECT_EQ(0xF00D, dst.rawbuf[0]);
  EXPECT_EQ(0xBEEF, dst.rawbuf[1]);
  EXPECT_EQ(0xDEAD, dst.rawbuf[test_size - 1]);
}

// Tests for decode().

// Test decode of a NEC message.
TEST(TestDecode, DecodeNEC) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendNEC(0x807F40BF);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(kNECBits, irsend.capture.bits);
  EXPECT_EQ(0x807F40BF, irsend.capture.value);
}

// Test decode of a JVC message.
TEST(TestDecode, DecodeJVC) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendJVC(0xC2B8);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(JVC, irsend.capture.decode_type);
  EXPECT_EQ(kJvcBits, irsend.capture.bits);
  EXPECT_EQ(0xC2B8, irsend.capture.value);
}

// Test decode of a LG message.
TEST(TestDecode, DecodeLG) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendLG(0x4B4AE51);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(LG, irsend.capture.decode_type);
  EXPECT_EQ(kLgBits, irsend.capture.bits);
  EXPECT_EQ(0x4B4AE51, irsend.capture.value);

  irsend.reset();
  irsend.sendLG(0xB4B4AE51, kLg32Bits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(LG, irsend.capture.decode_type);
  EXPECT_EQ(kLg32Bits, irsend.capture.bits);
  EXPECT_EQ(0xB4B4AE51, irsend.capture.value);
}

// Test decode of a Panasonic message.
TEST(TestDecode, DecodePanasonic) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
}

// Test decode of a Samsun message.
TEST(TestDecode, DecodeSamsung) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
}

// Test decode of a Sherwood message.
TEST(TestDecode, DecodeSherwood) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSherwood(0x807F40BF);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  // Sherwood codes are really NEC codes.
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(kNECBits, irsend.capture.bits);
  EXPECT_EQ(0x807F40BF, irsend.capture.value);
}

// Test decode of a Whynter message.
TEST(TestDecode, DecodeWhynter) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendWhynter(0x87654321);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(WHYNTER, irsend.capture.decode_type);
  EXPECT_EQ(kWhynterBits, irsend.capture.bits);
  EXPECT_EQ(0x87654321, irsend.capture.value);
}

// Test decode of Sony messages.
TEST(TestDecode, DecodeSony) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  // Synthesised Normal Sony 20-bit message.
  irsend.reset();
  irsend.sendSony(irsend.encodeSony(kSony20Bits, 0x1, 0x1, 0x1));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SONY, irsend.capture.decode_type);
  EXPECT_EQ(kSony20Bits, irsend.capture.bits);
  EXPECT_EQ(0x81080, irsend.capture.value);

  // Synthesised Normal Sony 15-bit message.
  irsend.reset();
  irsend.sendSony(irsend.encodeSony(kSony15Bits, 21, 1), kSony15Bits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SONY, irsend.capture.decode_type);
  EXPECT_EQ(kSony15Bits, irsend.capture.bits);
  EXPECT_EQ(0x5480, irsend.capture.value);

  // Synthesised Normal Sony 12-bit message.
  irsend.reset();
  irsend.sendSony(irsend.encodeSony(kSony12Bits, 21, 1), kSony12Bits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SONY, irsend.capture.decode_type);
  EXPECT_EQ(kSony12Bits, irsend.capture.bits);
  EXPECT_EQ(0xA90, irsend.capture.value);
}

// Test decode of Sharp messages.
TEST(TestDecode, DecodeSharp) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSharpRaw(0x454A);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SHARP, irsend.capture.decode_type);
  EXPECT_EQ(kSharpBits, irsend.capture.bits);
  EXPECT_EQ(0x454A, irsend.capture.value);
}

// Test decode of Sanyo messages.
TEST(TestDecode, DecodeSanyo) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSanyoLC7461(0x2468DCB56A9);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SANYO_LC7461, irsend.capture.decode_type);
  EXPECT_EQ(kSanyoLC7461Bits, irsend.capture.bits);
  EXPECT_EQ(0x2468DCB56A9, irsend.capture.value);
}

// Test decode of RC-MM messages.
TEST(TestDecode, DecodeRCMM) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  // Normal RCMM 24-bit message.
  irsend.reset();
  irsend.sendRCMM(0xe0a600);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RCMM, irsend.capture.decode_type);
  EXPECT_EQ(kRCMMBits, irsend.capture.bits);
  EXPECT_EQ(0xe0a600, irsend.capture.value);

  // Normal RCMM 12-bit message.
  irsend.reset();
  irsend.sendRCMM(0x600, 12);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RCMM, irsend.capture.decode_type);
  EXPECT_EQ(12, irsend.capture.bits);
  EXPECT_EQ(0x600, irsend.capture.value);

  // Normal RCMM 32-bit message.
  irsend.reset();
  irsend.sendRCMM(0x28e0a600, 32);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RCMM, irsend.capture.decode_type);
  EXPECT_EQ(32, irsend.capture.bits);
  EXPECT_EQ(0x28e0a600, irsend.capture.value);
}

// Test decode of Mitsubishi messages.
TEST(TestDecode, DecodeMitsubishi) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendMitsubishi(0xC2B8);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MITSUBISHI, irsend.capture.decode_type);
  EXPECT_EQ(kMitsubishiBits, irsend.capture.bits);
  EXPECT_EQ(0xC2B8, irsend.capture.value);
}

// Test decode of RC-5/RC-5X messages.
TEST(TestDecode, DecodeRC5) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  // Normal RC-5 12-bit message.
  irsend.reset();
  irsend.sendRC5(0x175);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(kRC5Bits, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);
  // Synthesised Normal RC-5X 13-bit message.
  irsend.reset();
  irsend.sendRC5(irsend.encodeRC5X(0x02, 0x41, true), kRC5XBits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC5X, irsend.capture.decode_type);
  EXPECT_EQ(kRC5XBits, irsend.capture.bits);
  EXPECT_EQ(0x1881, irsend.capture.value);
}

// Test decode of RC-6 messages.
TEST(TestDecode, DecodeRC6) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  // Normal RC-6 Mode 0 (20-bit) message.
  irsend.reset();
  irsend.sendRC6(0x175);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC6, irsend.capture.decode_type);
  EXPECT_EQ(kRC6Mode0Bits, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);

  // Normal RC-6 36-bit message.
  irsend.reset();
  irsend.sendRC6(0xC800F742A, kRC6_36Bits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC6, irsend.capture.decode_type);
  EXPECT_EQ(kRC6_36Bits, irsend.capture.bits);
  EXPECT_EQ(0xC800F742A, irsend.capture.value);
}

// Test decode of Dish messages.
TEST(TestDecode, DecodeDish) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendDISH(0x9C00);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DISH, irsend.capture.decode_type);
  EXPECT_EQ(kDishBits, irsend.capture.bits);
  EXPECT_EQ(0x9C00, irsend.capture.value);
}

// Test decode of Denon messages.
TEST(TestDecode, DecodeDenon) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  // Normal Denon 15-bit message. (Sharp)
  irsend.reset();
  irsend.sendDenon(0x2278);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DENON, irsend.capture.decode_type);
  EXPECT_EQ(kDenonBits, irsend.capture.bits);
  EXPECT_EQ(0x2278, irsend.capture.value);
  // Legacy Denon 14-bit message.
  irsend.reset();
  irsend.sendDenon(0x1278, kDenonLegacyBits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DENON, irsend.capture.decode_type);
  EXPECT_EQ(kDenonBits, irsend.capture.bits);
  EXPECT_EQ(0x1278, irsend.capture.value);
  // Normal Denon 48-bit message. (Panasonic/Kaseikyo)
  irsend.reset();
  irsend.sendDenon(0x2A4C028D6CE3, kDenon48Bits);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DENON, irsend.capture.decode_type);
  EXPECT_EQ(kDenon48Bits, irsend.capture.bits);
  EXPECT_EQ(0x2A4C028D6CE3, irsend.capture.value);
}

// Test decode of Coolix messages.
TEST(TestDecode, DecodeCoolix) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendCOOLIX(0x123456);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(COOLIX, irsend.capture.decode_type);
  EXPECT_EQ(kCoolixBits, irsend.capture.bits);
  EXPECT_EQ(0x123456, irsend.capture.value);
}

// Test decode of Aiwa messages.
TEST(TestDecode, DecodeAiwa) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendAiwaRCT501(0x7F);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(AIWA_RC_T501, irsend.capture.decode_type);
  EXPECT_EQ(kAiwaRcT501Bits, irsend.capture.bits);
  EXPECT_EQ(0x7F, irsend.capture.value);
}

// Test matchData() on space encoded data.
TEST(TestMatchData, SpaceEncoded) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t space_encoded_raw[11] = {500, 500,  500, 1500, 499, 499,
                                    501, 1501, 499, 1490, 500};
  match_result_t result;

  irsend.reset();
  irsend.sendRaw(space_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 500, 1500, 500, 500);
  ASSERT_TRUE(result.success);
  EXPECT_EQ(0b01011, result.data);
  EXPECT_EQ(10, result.used);

  irsend.reset();
  irsend.sendRaw(space_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 500, 1000, 500, 500);
  ASSERT_FALSE(result.success);
}

// Test matchData() on mark encoded data.
TEST(TestMatchData, MarkEncoded) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t mark_encoded_raw[11] = {500,  500, 1500, 500, 499, 499,
                                   1501, 501, 1499, 490, 500};
  match_result_t result;

  irsend.reset();
  irsend.sendRaw(mark_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  // MSBF order.
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1500, 500, 500, 500);
  ASSERT_TRUE(result.success);
  EXPECT_EQ(0b01011, result.data);
  EXPECT_EQ(10, result.used);
  // LSBF order.
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1500, 500, 500, 500,
                            kTolerance, kMarkExcess, false);
  ASSERT_TRUE(result.success);
  EXPECT_EQ(0b11010, result.data);  // Bits reversed of the previous test.
  EXPECT_EQ(10, result.used);

  irsend.reset();
  irsend.sendRaw(mark_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  // MSBF order.
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1000, 500, 500, 500);
  ASSERT_FALSE(result.success);
  // LSBF order.
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1000, 500, 500, 500,
                            kTolerance, kMarkExcess, false);
  ASSERT_FALSE(result.success);
}

// Test matchData() on "equal total bit time" encoded data.
TEST(TestMatchData, EqualTotalBitTimeEncoded) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t equal_encoded_raw[11] = {500,  1500, 1500, 500, 499, 1499,
                                    1501, 501,  1499, 490, 500};
  match_result_t result;

  irsend.reset();
  irsend.sendRaw(equal_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1500, 500, 500, 1500);
  ASSERT_TRUE(result.success);
  EXPECT_EQ(0b01011, result.data);
  EXPECT_EQ(10, result.used);

  irsend.reset();
  irsend.sendRaw(equal_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1000, 500, 500, 1000);
  ASSERT_FALSE(result.success);
}

// Test matchData() on arbitrary encoded data.
TEST(TestMatchData, ArbitraryEncoded) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t arbitrary_encoded_raw[11] = {500,  1500, 3000, 1000, 499, 1499,
                                        3001, 1001, 2999, 990,  500};
  match_result_t result;

  irsend.reset();
  irsend.sendRaw(arbitrary_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  result =
      irrecv.matchData(irsend.capture.rawbuf + 1, 5, 3000, 1000, 500, 1500);
  ASSERT_TRUE(result.success);
  EXPECT_EQ(0b01011, result.data);
  EXPECT_EQ(10, result.used);

  irsend.reset();
  irsend.sendRaw(arbitrary_encoded_raw, 11, 38000);
  irsend.makeDecodeResult();
  result = irrecv.matchData(irsend.capture.rawbuf + 1, 5, 1000, 500, 500, 1000);
  ASSERT_FALSE(result.success);
}

TEST(TestMatchGeneric, NormalWithNoAtleast) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t good_entries_trailing_space = 12;
  uint16_t good_trailing_space_data[good_entries_trailing_space] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000,  // Footer mark
      15000};  // Footer space

  uint16_t good_entries_no_trailing_space = 11;
  uint16_t good_no_trailing_space_data[good_entries_no_trailing_space] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000};  // Footer mark  (No Footer space.)

  uint16_t offset = kStartOffset;
  irsend.reset();
  irsend.sendRaw(good_trailing_space_data, good_entries_trailing_space, 38000);
  irsend.makeDecodeResult();
  uint64_t result_data = 0;
  uint16_t entries_used = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      false,  // No atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(good_entries_trailing_space, entries_used);

  // Same again but with a footer space mis-match, which should fail.
  result_data = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 30000,  // Footer
      false,  // no atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Same again as first part but with no footer space data as the last entry.
  irsend.reset();
  result_data = 0;
  irsend.sendRaw(good_no_trailing_space_data, good_entries_no_trailing_space,
                 38000);
  irsend.makeDecodeResult();
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      false,  // No atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(good_entries_no_trailing_space, entries_used);
}


TEST(TestMatchGeneric, NormalWithAtleast) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t good_entries_trailing_space = 12;
  uint16_t good_trailing_space_data[good_entries_trailing_space] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000,  // Footer mark
      15000};  // Footer space

  uint16_t good_entries_no_trailing_space = 11;
  uint16_t good_no_trailing_space_data[good_entries_no_trailing_space] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000};  // Footer mark  (No Footer space.)

  uint16_t offset = kStartOffset;
  irsend.reset();
  irsend.sendRaw(good_trailing_space_data, good_entries_trailing_space, 38000);
  irsend.makeDecodeResult();
  uint64_t result_data = 0;
  uint16_t entries_used = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(good_entries_trailing_space, entries_used);

  // Same again but with a footer space under-match.
  result_data = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 1500,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(good_entries_trailing_space, entries_used);

  // Same again but with a footer space under-match using less bits so the
  // atleast footer isn't the last entry in the buffer.
  result_data = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      3,  // nbits (1 less than normal)
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      500, 500,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b101, result_data);
  // -2 because we reduced nbits by 1.
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset - 2, entries_used);
  EXPECT_EQ(good_entries_trailing_space - 2, entries_used);

  // Same again but with a footer space over-match, which should fail.
  result_data = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      3,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      500, 10000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Same as first part but with no footer space data as the last entry.
  irsend.reset();
  result_data = 0;
  irsend.sendRaw(good_no_trailing_space_data, good_entries_no_trailing_space,
                 38000);
  irsend.makeDecodeResult();
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(good_entries_no_trailing_space, entries_used);
}

TEST(TestMatchGeneric, FailureCases) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t entries = 11;
  uint16_t data[entries] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000};  // Footer mark  (No Footer space.)

  uint16_t offset = kStartOffset;
  irsend.reset();
  irsend.sendRaw(data, entries, 38000);
  irsend.makeDecodeResult();
  uint16_t entries_used = 0;

  // Wanting too many bits should fail.
  uint64_t result_data = 0;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      5,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad header mark.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      2000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad header space.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 2000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad one mark.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      600, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad one space.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2500,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad zero space.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1500,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad zero mark.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      900, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Bad Footer mark.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      1000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Confirm it really does match as expected..
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
}

TEST(TestMatchGeneric, MissingHeaderFooter) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t entries = 11;
  uint16_t data[entries] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000};  // Footer mark  (No Footer space.)

  uint16_t offset = kStartOffset;
  irsend.reset();
  irsend.sendRaw(data, entries, 38000);
  irsend.makeDecodeResult();
  uint16_t entries_used = 0;

  uint64_t result_data = 0;

  // No footer match
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // NO Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset - 1, entries_used);
  EXPECT_EQ(entries - 1, entries_used);

  // No header match (should fail)
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      0, 0,  // NO Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // NO Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // No header match but starting after header
  offset += 2;
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      0, 0,  // NO Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - offset, entries_used);
  EXPECT_EQ(entries - 2, entries_used);
}

TEST(TestMatchGeneric, BitOrdering) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t entries = 11;
  uint16_t data[entries] = {
      8000,  // Header mark
      4000,  // Header space
      500, 2000,  // Bit #0 (1)
      500, 1000,  // Bit #1 (0)
      500, 2000,  // Bit #2 (1)
      500, 1000,  // Bit #3 (0)
      3000};  // Footer mark  (No Footer space.)

  uint16_t offset = kStartOffset;
  irsend.reset();
  irsend.sendRaw(data, entries, 38000);
  irsend.makeDecodeResult();
  uint16_t entries_used = 0;

  uint64_t result_data = 0;

  // MSB order
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b1010, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(entries, entries_used);

  // LSB order
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, &result_data,
      irsend.capture.rawlen - offset,
      4,  // nbits
      8000, 4000,  // Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      3000, 15000,  // Footer
      true,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      false);  // LSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b0101, result_data);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(entries, entries_used);
}

TEST(TestMatchGeneric, UsingBytes) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  uint16_t entries = 32;
  uint16_t data[entries] = {
      // No header
      500, 2000,   // Byte #0 Bit #0 (1)
      500, 1000,   // Byte #0 Bit #1 (0)
      500, 2000,   // Byte #0 Bit #2 (1)
      500, 1000,   // Byte #0 Bit #3 (0)
      500, 2000,   // Byte #0 Bit #4 (1)
      500, 1000,   // Byte #0 Bit #5 (0)
      500, 2000,   // Byte #0 Bit #6 (1)
      500, 1000,   // Byte #0 Bit #7 (0)
      500, 2000,   // Byte #1 Bit #0 (1)
      500, 2000,   // Byte #1 Bit #1 (1)
      500, 2000,   // Byte #1 Bit #2 (1)
      500, 2000,   // Byte #1 Bit #3 (1)
      500, 1000,   // Byte #1 Bit #4 (0)
      500, 1000,   // Byte #1 Bit #5 (0)
      500, 1000,   // Byte #1 Bit #6 (0)
      500, 1000};  // Byte #1 Bit #7 (0) & No footer

  uint16_t offset = kStartOffset;
  irsend.reset();
  irsend.sendRaw(data, entries, 38000);
  irsend.makeDecodeResult();
  uint16_t entries_used = 0;

  uint8_t result_data[4] = {};  // Bigger than we need.

  // MSB order
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, result_data,
      irsend.capture.rawlen - offset,
      2 * 8,  // nbits
      0, 0,  // No Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // No Footer
      false,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b10101010, result_data[0]);
  EXPECT_EQ(0b11110000, result_data[1]);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(entries, entries_used);

  // LSB order
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, result_data,
      irsend.capture.rawlen - offset,
      2 * 8,  // nbits
      0, 0,  // No Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // No Footer
      false,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      false);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b01010101, result_data[0]);
  EXPECT_EQ(0b00001111, result_data[1]);
  EXPECT_EQ(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(entries, entries_used);

  // Asking for too much.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, result_data,
      irsend.capture.rawlen - offset,
      3 * 8,  // nbits
      0, 0,  // No Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // No Footer
      false,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Asking for less than what is there.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, result_data,
      irsend.capture.rawlen - offset,
      1 * 8,  // nbits
      0, 0,  // No Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // No Footer
      false,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_NE(0, entries_used);
  EXPECT_EQ(0b10101010, result_data[0]);
  EXPECT_GT(irsend.capture.rawlen - kStartOffset, entries_used);
  EXPECT_EQ(16, entries_used);

  // Asking for non mod-8 size should fail.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, result_data,
      irsend.capture.rawlen - offset,
      9,  // nbits
      0, 0,  // No Header
      500, 2000,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // No Footer
      false,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);

  // Expecting different timings should fail.
  entries_used = irrecv.matchGeneric(
      irsend.capture.rawbuf + offset, result_data,
      irsend.capture.rawlen - offset,
      8,  // nbits
      0, 0,  // No Header
      500, 900,  // one mark & space
      500, 1000,  // zero mark & space
      0, 0,  // No Footer
      false,  // atleast on the footer space.
      1,  // 1% Tolerance
      0,  // No excess margin
      true);  // MSB first.
  ASSERT_EQ(0, entries_used);
}
