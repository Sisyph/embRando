//===- unittest/ProfileData/InstrProfTest.cpp -------------------------------=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ProfileData/InstrProfReader.h"
#include "llvm/ProfileData/InstrProfWriter.h"
#include "gtest/gtest.h"

#include <cstdarg>

using namespace llvm;

static ::testing::AssertionResult NoError(std::error_code EC) {
  if (!EC)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "error " << EC.value()
                                       << ": " << EC.message();
}

static ::testing::AssertionResult ErrorEquals(std::error_code Expected,
                                              std::error_code Found) {
  if (Expected == Found)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "error " << Found.value()
                                       << ": " << Found.message();
}

namespace {

struct InstrProfTest : ::testing::Test {
  InstrProfWriter Writer;
  std::unique_ptr<IndexedInstrProfReader> Reader;

  void readProfile(std::unique_ptr<MemoryBuffer> Profile) {
    auto ReaderOrErr = IndexedInstrProfReader::create(std::move(Profile));
    ASSERT_TRUE(NoError(ReaderOrErr.getError()));
    Reader = std::move(ReaderOrErr.get());
  }
};

TEST_F(InstrProfTest, write_and_read_empty_profile) {
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));
  ASSERT_TRUE(Reader->begin() == Reader->end());
}

TEST_F(InstrProfTest, write_and_read_one_function) {
  InstrProfRecord Record("foo", 0x1234, {1, 2, 3, 4});
  Writer.addRecord(std::move(Record));
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));

  auto I = Reader->begin(), E = Reader->end();
  ASSERT_TRUE(I != E);
  ASSERT_EQ(StringRef("foo"), I->Name);
  ASSERT_EQ(0x1234U, I->Hash);
  ASSERT_EQ(4U, I->Counts.size());
  ASSERT_EQ(1U, I->Counts[0]);
  ASSERT_EQ(2U, I->Counts[1]);
  ASSERT_EQ(3U, I->Counts[2]);
  ASSERT_EQ(4U, I->Counts[3]);
  ASSERT_TRUE(++I == E);
}

TEST_F(InstrProfTest, get_instr_prof_record) {
  InstrProfRecord Record1("foo", 0x1234, {1, 2});
  InstrProfRecord Record2("foo", 0x1235, {3, 4});
  Writer.addRecord(std::move(Record1));
  Writer.addRecord(std::move(Record2));
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));

  ErrorOr<InstrProfRecord> R = Reader->getInstrProfRecord("foo", 0x1234);
  ASSERT_TRUE(NoError(R.getError()));
  ASSERT_EQ(2U, R.get().Counts.size());
  ASSERT_EQ(1U, R.get().Counts[0]);
  ASSERT_EQ(2U, R.get().Counts[1]);

  R = Reader->getInstrProfRecord("foo", 0x1235);
  ASSERT_TRUE(NoError(R.getError()));
  ASSERT_EQ(2U, R.get().Counts.size());
  ASSERT_EQ(3U, R.get().Counts[0]);
  ASSERT_EQ(4U, R.get().Counts[1]);

  R = Reader->getInstrProfRecord("foo", 0x5678);
  ASSERT_TRUE(ErrorEquals(instrprof_error::hash_mismatch, R.getError()));

  R = Reader->getInstrProfRecord("bar", 0x1234);
  ASSERT_TRUE(ErrorEquals(instrprof_error::unknown_function, R.getError()));
}

TEST_F(InstrProfTest, get_function_counts) {
  InstrProfRecord Record1("foo", 0x1234, {1, 2});
  InstrProfRecord Record2("foo", 0x1235, {3, 4});
  Writer.addRecord(std::move(Record1));
  Writer.addRecord(std::move(Record2));
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));

  std::vector<uint64_t> Counts;
  ASSERT_TRUE(NoError(Reader->getFunctionCounts("foo", 0x1234, Counts)));
  ASSERT_EQ(2U, Counts.size());
  ASSERT_EQ(1U, Counts[0]);
  ASSERT_EQ(2U, Counts[1]);

  ASSERT_TRUE(NoError(Reader->getFunctionCounts("foo", 0x1235, Counts)));
  ASSERT_EQ(2U, Counts.size());
  ASSERT_EQ(3U, Counts[0]);
  ASSERT_EQ(4U, Counts[1]);

  std::error_code EC;
  EC = Reader->getFunctionCounts("foo", 0x5678, Counts);
  ASSERT_TRUE(ErrorEquals(instrprof_error::hash_mismatch, EC));

  EC = Reader->getFunctionCounts("bar", 0x1234, Counts);
  ASSERT_TRUE(ErrorEquals(instrprof_error::unknown_function, EC));
}

TEST_F(InstrProfTest, get_icall_data_read_write) {
  InstrProfRecord Record1("caller", 0x1234, {1, 2});
  InstrProfRecord Record2("callee1", 0x1235, {3, 4});
  InstrProfRecord Record3("callee2", 0x1235, {3, 4});
  InstrProfRecord Record4("callee3", 0x1235, {3, 4});

  // 4 value sites.
  Record1.reserveSites(IPVK_IndirectCallTarget, 4);
  InstrProfValueData VD0[] = {{(uint64_t) "callee1", 1},
                              {(uint64_t) "callee2", 2},
                              {(uint64_t) "callee3", 3}};
  Record1.addValueData(IPVK_IndirectCallTarget, 0, VD0, 3, 0);
  // No valeu profile data at the second site.
  Record1.addValueData(IPVK_IndirectCallTarget, 1, 0, 0, 0);
  InstrProfValueData VD2[] = {{(uint64_t) "callee1", 1},
                              {(uint64_t) "callee2", 2}};
  Record1.addValueData(IPVK_IndirectCallTarget, 2, VD2, 2, 0);
  InstrProfValueData VD3[] = {{(uint64_t) "callee1", 1}};
  Record1.addValueData(IPVK_IndirectCallTarget, 3, VD3, 1, 0);

  Writer.addRecord(std::move(Record1));
  Writer.addRecord(std::move(Record2));
  Writer.addRecord(std::move(Record3));
  Writer.addRecord(std::move(Record4));
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));

  ErrorOr<InstrProfRecord> R = Reader->getInstrProfRecord("caller", 0x1234);
  ASSERT_TRUE(NoError(R.getError()));
  ASSERT_EQ(4U, R.get().getNumValueSites(IPVK_IndirectCallTarget));
  ASSERT_EQ(3U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 0));
  ASSERT_EQ(0U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 1));
  ASSERT_EQ(2U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 2));
  ASSERT_EQ(1U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 3));

  std::unique_ptr<InstrProfValueData[]> VD =
      R.get().getValueForSite(IPVK_IndirectCallTarget, 0);
  // Now sort the target acording to frequency.
  std::sort(&VD[0], &VD[3],
            [](const InstrProfValueData &VD1, const InstrProfValueData &VD2) {
              return VD1.Count > VD2.Count;
            });
  ASSERT_EQ(StringRef((const char *)VD[0].Value, 7), StringRef("callee3"));
  ASSERT_EQ(StringRef((const char *)VD[1].Value, 7), StringRef("callee2"));
  ASSERT_EQ(StringRef((const char *)VD[2].Value, 7), StringRef("callee1"));
}

TEST_F(InstrProfTest, get_icall_data_merge1) {
  InstrProfRecord Record11("caller", 0x1234, {1, 2});
  InstrProfRecord Record12("caller", 0x1234, {1, 2});
  InstrProfRecord Record2("callee1", 0x1235, {3, 4});
  InstrProfRecord Record3("callee2", 0x1235, {3, 4});
  InstrProfRecord Record4("callee3", 0x1235, {3, 4});
  InstrProfRecord Record5("callee3", 0x1235, {3, 4});
  InstrProfRecord Record6("callee4", 0x1235, {3, 5});

  // 5 value sites.
  Record11.reserveSites(IPVK_IndirectCallTarget, 5);
  InstrProfValueData VD0[] = {{(uint64_t) "callee1", 1},
                              {(uint64_t) "callee2", 2},
                              {(uint64_t) "callee3", 3},
                              {(uint64_t) "callee4", 4}};
  Record11.addValueData(IPVK_IndirectCallTarget, 0, VD0, 4, 0);

  // No valeu profile data at the second site.
  Record11.addValueData(IPVK_IndirectCallTarget, 1, 0, 0, 0);

  InstrProfValueData VD2[] = {{(uint64_t) "callee1", 1},
                              {(uint64_t) "callee2", 2},
                              {(uint64_t) "callee3", 3}};
  Record11.addValueData(IPVK_IndirectCallTarget, 2, VD2, 3, 0);

  InstrProfValueData VD3[] = {{(uint64_t) "callee1", 1}};
  Record11.addValueData(IPVK_IndirectCallTarget, 3, VD3, 1, 0);

  InstrProfValueData VD4[] = {{(uint64_t) "callee1", 1},
                              {(uint64_t) "callee2", 2},
                              {(uint64_t) "callee3", 3}};
  Record11.addValueData(IPVK_IndirectCallTarget, 4, VD4, 3, 0);

  // A differnt record for the same caller.
  Record12.reserveSites(IPVK_IndirectCallTarget, 5);
  InstrProfValueData VD02[] = {{(uint64_t) "callee2", 5},
                               {(uint64_t) "callee3", 3}};
  Record12.addValueData(IPVK_IndirectCallTarget, 0, VD02, 2, 0);

  // No valeu profile data at the second site.
  Record12.addValueData(IPVK_IndirectCallTarget, 1, 0, 0, 0);

  InstrProfValueData VD22[] = {{(uint64_t) "callee2", 1},
                               {(uint64_t) "callee3", 3},
                               {(uint64_t) "callee4", 4}};
  Record12.addValueData(IPVK_IndirectCallTarget, 2, VD22, 3, 0);

  Record12.addValueData(IPVK_IndirectCallTarget, 3, 0, 0, 0);

  InstrProfValueData VD42[] = {{(uint64_t) "callee1", 1},
                               {(uint64_t) "callee2", 2},
                               {(uint64_t) "callee3", 3}};
  Record12.addValueData(IPVK_IndirectCallTarget, 4, VD42, 3, 0);

  Writer.addRecord(std::move(Record11));
  // Merge profile data.
  Writer.addRecord(std::move(Record12));

  Writer.addRecord(std::move(Record2));
  Writer.addRecord(std::move(Record3));
  Writer.addRecord(std::move(Record4));
  Writer.addRecord(std::move(Record5));
  Writer.addRecord(std::move(Record6));
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));

  ErrorOr<InstrProfRecord> R = Reader->getInstrProfRecord("caller", 0x1234);
  ASSERT_TRUE(NoError(R.getError()));
  ASSERT_EQ(5U, R.get().getNumValueSites(IPVK_IndirectCallTarget));
  ASSERT_EQ(4U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 0));
  ASSERT_EQ(0U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 1));
  ASSERT_EQ(4U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 2));
  ASSERT_EQ(1U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 3));
  ASSERT_EQ(3U, R.get().getNumValueDataForSite(IPVK_IndirectCallTarget, 4));

  std::unique_ptr<InstrProfValueData[]> VD =
      R.get().getValueForSite(IPVK_IndirectCallTarget, 0);
  // Now sort the target acording to frequency.
  std::sort(&VD[0], &VD[4],
            [](const InstrProfValueData &VD1, const InstrProfValueData &VD2) {
              return VD1.Count > VD2.Count;
            });
  ASSERT_EQ(StringRef((const char *)VD[0].Value, 7), StringRef("callee2"));
  ASSERT_EQ(7U, VD[0].Count);
  ASSERT_EQ(StringRef((const char *)VD[1].Value, 7), StringRef("callee3"));
  ASSERT_EQ(6U, VD[1].Count);
  ASSERT_EQ(StringRef((const char *)VD[2].Value, 7), StringRef("callee4"));
  ASSERT_EQ(4U, VD[2].Count);
  ASSERT_EQ(StringRef((const char *)VD[3].Value, 7), StringRef("callee1"));
  ASSERT_EQ(1U, VD[3].Count);

  std::unique_ptr<InstrProfValueData[]> VD_2(
      R.get().getValueForSite(IPVK_IndirectCallTarget, 2));
  std::sort(&VD_2[0], &VD_2[4],
            [](const InstrProfValueData &VD1, const InstrProfValueData &VD2) {
              return VD1.Count > VD2.Count;
            });
  ASSERT_EQ(StringRef((const char *)VD_2[0].Value, 7), StringRef("callee3"));
  ASSERT_EQ(6U, VD_2[0].Count);
  ASSERT_EQ(StringRef((const char *)VD_2[1].Value, 7), StringRef("callee4"));
  ASSERT_EQ(4U, VD_2[1].Count);
  ASSERT_EQ(StringRef((const char *)VD_2[2].Value, 7), StringRef("callee2"));
  ASSERT_EQ(3U, VD_2[2].Count);
  ASSERT_EQ(StringRef((const char *)VD_2[3].Value, 7), StringRef("callee1"));
  ASSERT_EQ(1U, VD_2[3].Count);

  std::unique_ptr<InstrProfValueData[]> VD_3(
      R.get().getValueForSite(IPVK_IndirectCallTarget, 3));
  ASSERT_EQ(StringRef((const char *)VD_3[0].Value, 7), StringRef("callee1"));
  ASSERT_EQ(1U, VD_3[0].Count);

  std::unique_ptr<InstrProfValueData[]> VD_4(
      R.get().getValueForSite(IPVK_IndirectCallTarget, 4));
  std::sort(&VD_4[0], &VD_4[3],
            [](const InstrProfValueData &VD1, const InstrProfValueData &VD2) {
              return VD1.Count > VD2.Count;
            });
  ASSERT_EQ(StringRef((const char *)VD_4[0].Value, 7), StringRef("callee3"));
  ASSERT_EQ(6U, VD_4[0].Count);
  ASSERT_EQ(StringRef((const char *)VD_4[1].Value, 7), StringRef("callee2"));
  ASSERT_EQ(4U, VD_4[1].Count);
  ASSERT_EQ(StringRef((const char *)VD_4[2].Value, 7), StringRef("callee1"));
  ASSERT_EQ(2U, VD_4[2].Count);
}

TEST_F(InstrProfTest, get_max_function_count) {
  InstrProfRecord Record1("foo", 0x1234, {1ULL << 31, 2});
  InstrProfRecord Record2("bar", 0, {1ULL << 63});
  InstrProfRecord Record3("baz", 0x5678, {0, 0, 0, 0});
  Writer.addRecord(std::move(Record1));
  Writer.addRecord(std::move(Record2));
  Writer.addRecord(std::move(Record3));
  auto Profile = Writer.writeBuffer();
  readProfile(std::move(Profile));

  ASSERT_EQ(1ULL << 63, Reader->getMaximumFunctionCount());
}

} // end anonymous namespace