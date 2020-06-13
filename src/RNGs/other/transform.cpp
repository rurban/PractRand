#include <string>
#include <sstream>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/other/transform.h"

namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				void Transform64::seed(Uint64 s) {base_rng->seed(s);}
				Uint64 Transform64::get_flags() const {return base_rng->get_flags() | FLAG::USES_INDIRECTION;}
				Transform64::~Transform64() {delete base_rng;}
				void Transform32::seed(Uint64 s) {base_rng->seed(s);}
				Uint64 Transform32::get_flags() const {return base_rng->get_flags() | FLAG::USES_INDIRECTION;}
				Transform32::~Transform32() {delete base_rng;}
				void Transform16::seed(Uint64 s) {base_rng->seed(s);}
				Uint64 Transform16::get_flags() const {return base_rng->get_flags() | FLAG::USES_INDIRECTION;}
				Transform16::~Transform16() {delete base_rng;}
				void Transform8::seed(Uint64 s) {base_rng->seed(s);}
				Uint64 Transform8::get_flags() const {return base_rng->get_flags() | FLAG::USES_INDIRECTION;}
				Transform8::~Transform8() {delete base_rng;}

				void GeneralizedTableTransform::seed(Uint64 s) {base_rng->seed(s);}
				Uint64 GeneralizedTableTransform::get_flags() const {
					return base_rng->get_flags() | FLAG::USES_FLOW_CONTROL | FLAG::STATE_UNAVAILABLE;//not exactly, but close enough
				}
				GeneralizedTableTransform::~GeneralizedTableTransform() {delete base_rng;}
				void GeneralizedTableTransform::walk_state(StateWalkingObject *walker) {
					base_rng->walk_state(walker);
					buf_data = 0;
					buf_count = 0;
					finished_bytes.clear();
				}
				Uint8 GeneralizedTableTransform::raw8() {
					while (true) {
						if (!finished_bytes.empty()) {
							Uint8 rv = finished_bytes.front();
							finished_bytes.pop_front();
							return rv;
						}
						Uint64 in = base_rng->raw64();
						for (int i = 0; i < 8; i++) {
							const Entry &e = table[in & 255];
							in >>= 8;
							buf_data |= Uint32(e.data) << buf_count;
							buf_count += e.count;
							if (buf_count >= 8) {
								finished_bytes.push_back(buf_data & 255);
								buf_count -= 8;
								buf_data >>= 8;
							}
						}
					}
				}
				static const GeneralizedTableTransform::Entry self_shrinking_table11[256] = {
					{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },
					{ 0, 2 },{ 0, 2 },{ 0, 3 },{ 1, 3 },{ 1, 2 },{ 1, 2 },{ 2, 3 },{ 3, 3 },
					{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 2, 2 },{ 2, 2 },{ 4, 3 },{ 5, 3 },{ 3, 2 },{ 3, 2 },{ 6, 3 },{ 7, 3 },
					{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 0, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },
					{ 0, 2 },{ 0, 2 },{ 0, 3 },{ 1, 3 },{ 1, 2 },{ 1, 2 },{ 2, 3 },{ 3, 3 },
					{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 2, 2 },{ 2, 2 },{ 4, 3 },{ 5, 3 },{ 3, 2 },{ 3, 2 },{ 6, 3 },{ 7, 3 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },
					{ 0, 2 },{ 0, 2 },{ 0, 3 },{ 1, 3 },{ 1, 2 },{ 1, 2 },{ 2, 3 },{ 3, 3 },
					{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },{ 0, 1 },{ 0, 1 },{ 0, 2 },{ 1, 2 },
					{ 0, 2 },{ 0, 2 },{ 0, 3 },{ 1, 3 },{ 1, 2 },{ 1, 2 },{ 2, 3 },{ 3, 3 },
					{ 0, 2 },{ 0, 2 },{ 0, 3 },{ 1, 3 },{ 0, 2 },{ 0, 2 },{ 0, 3 },{ 1, 3 },
					{ 0, 3 },{ 0, 3 },{ 0, 4 },{ 1, 4 },{ 1, 3 },{ 1, 3 },{ 2, 4 },{ 3, 4 },
					{ 1, 2 },{ 1, 2 },{ 2, 3 },{ 3, 3 },{ 1, 2 },{ 1, 2 },{ 2, 3 },{ 3, 3 },
					{ 2, 3 },{ 2, 3 },{ 4, 4 },{ 5, 4 },{ 3, 3 },{ 3, 3 },{ 6, 4 },{ 7, 4 },
					{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 2, 2 },{ 2, 2 },{ 4, 3 },{ 5, 3 },{ 3, 2 },{ 3, 2 },{ 6, 3 },{ 7, 3 },
					{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },{ 1, 1 },{ 1, 1 },{ 2, 2 },{ 3, 2 },
					{ 2, 2 },{ 2, 2 },{ 4, 3 },{ 5, 3 },{ 3, 2 },{ 3, 2 },{ 6, 3 },{ 7, 3 },
					{ 2, 2 },{ 2, 2 },{ 4, 3 },{ 5, 3 },{ 2, 2 },{ 2, 2 },{ 4, 3 },{ 5, 3 },
					{ 4, 3 },{ 4, 3 },{ 8, 4 },{ 9, 4 },{ 5, 3 },{ 5, 3 },{10, 4 },{11, 4 },
					{ 3, 2 },{ 3, 2 },{ 6, 3 },{ 7, 3 },{ 3, 2 },{ 3, 2 },{ 6, 3 },{ 7, 3 },
					{ 6, 3 },{ 6, 3 },{12, 4 },{13, 4 },{ 7, 3 },{ 7, 3 },{14, 4 },{15, 4 }
				};
				std::string GeneralizedTableTransform::get_name() const {return name;}
				vRNG *apply_SelfShrinkTransform(vRNG *base_rng) {
					return new GeneralizedTableTransform (base_rng, self_shrinking_table11, std::string("[SShrink]") + base_rng->get_name() );
				}
				//vRNG *apply_SimpleShrinkTransform(vRNG *base_rng) {
				//	return new GeneralizedTableTransform (base_rng, NULL, std::string("[Shrink3of4]") + base_rng->get_name() );
				//}



				Uint64 BaysDurhamShuffle64::raw64() {
					Uint64 &storage = table[prev];
					Uint64 rv = storage;
					storage = base_rng->raw64();
					prev = Uint8(storage >> index_shift) & index_mask;
					return rv;
				}
				void BaysDurhamShuffle64::seed(Uint64 s) {
					base_rng->seed(s);
					for (int i = 0; i <= index_mask; i++) 
						table[i] = base_rng->raw64();
					for (int i = 0; i <= index_mask; i++) {raw64();raw64();}
				}
				void BaysDurhamShuffle64::walk_state(StateWalkingObject *walker) {
					base_rng->walk_state(walker);
					if (!(walker->get_properties() && walker->FLAG_CLUMSY)) {
						walker->handle(index_mask);
						walker->handle(index_shift);
					}
					for (int i = 0; i <= index_mask; i++) walker->handle(table[i]);
					walker->handle(prev);
					prev &= index_mask;
				}
				std::string BaysDurhamShuffle64::get_name() const {
					std::ostringstream tmp;
					tmp << "[BDS" << (1+int(index_mask)) << "]" << base_rng->get_name();
					return tmp.str();
				}

				Uint32 BaysDurhamShuffle32::raw32() {
					Uint32 &storage = table[prev];
					Uint32 rv = storage;
					storage = base_rng->raw32();
					prev = Uint8(storage >> index_shift) & index_mask;
					return rv;
				}
				void BaysDurhamShuffle32::seed(Uint64 s) {
					base_rng->seed(s);
					for (int i = 0; i <= index_mask; i++) 
						table[i] = base_rng->raw32();
					for (int i = 0; i <= index_mask; i++)  {raw32();raw32();}
				}
				void BaysDurhamShuffle32::walk_state(StateWalkingObject *walker) {
					base_rng->walk_state(walker);
					if (!(walker->get_properties() && walker->FLAG_CLUMSY)) {
						walker->handle(index_mask);
						walker->handle(index_shift);
					}
					for (int i = 0; i <= index_mask; i++) walker->handle(table[i]);
					walker->handle(prev);
					prev &= index_mask;
				}
				std::string BaysDurhamShuffle32::get_name() const {
					std::ostringstream tmp;
					tmp << "[BDS" << (1+int(index_mask)) << "]" << base_rng->get_name();
					return tmp.str();
				}

				Uint16 BaysDurhamShuffle16::raw16() {
					Uint16 &storage = table[prev];
					Uint16 rv = storage;
					storage = base_rng->raw32();
					prev = Uint8(storage >> index_shift) & index_mask;
					return rv;
				}
				void BaysDurhamShuffle16::seed(Uint64 s) {
					base_rng->seed(s);
					for (int i = 0; i <= index_mask; i++) 
						table[i] = base_rng->raw32();
					for (int i = 0; i <= index_mask; i++)  {raw16();raw16();}
				}
				void BaysDurhamShuffle16::walk_state(StateWalkingObject *walker) {
					base_rng->walk_state(walker);
					if (!(walker->get_properties() && walker->FLAG_CLUMSY)) {
						walker->handle(index_mask);
						walker->handle(index_shift);
					}
					for (int i = 0; i <= index_mask; i++) walker->handle(table[i]);
					walker->handle(prev);
					prev &= index_mask;
				}
				std::string BaysDurhamShuffle16::get_name() const {
					std::ostringstream tmp;
					tmp << "[BDS" << (1+int(index_mask)) << "]" << base_rng->get_name();
					return tmp.str();
				}

				Uint8 BaysDurhamShuffle8::raw8() {
					Uint8 &storage = table[prev];
					Uint8 rv = storage;
					storage = base_rng->raw32();
					prev = Uint8(storage >> index_shift) & index_mask;
					return rv;
				}
				void BaysDurhamShuffle8::seed(Uint64 s) {
					base_rng->seed(s);
					for (int i = 0; i <= index_mask; i++) 
						table[i] = base_rng->raw32();
					for (int i = 0; i <= index_mask; i++)  {raw8();raw8();}
				}
				void BaysDurhamShuffle8::walk_state(StateWalkingObject *walker) {
					base_rng->walk_state(walker);
					if (!(walker->get_properties() && walker->FLAG_CLUMSY)) {
						walker->handle(index_mask);
						walker->handle(index_shift);
					}
					for (int i = 0; i <= index_mask; i++) walker->handle(table[i]);
					walker->handle(prev);
					prev &= index_mask;
				}
				std::string BaysDurhamShuffle8::get_name() const {
					std::ostringstream tmp;
					tmp << "[BDS" << (1+int(index_mask)) << "]" << base_rng->get_name();
					return tmp.str();
				}
				vRNG *apply_BaysDurhamShuffle(vRNG *base_rng, int table_size_L2, int shift) {
					vRNG8 *tmp8 = dynamic_cast<vRNG8*>(base_rng);
					if (tmp8)  return new BaysDurhamShuffle8 (tmp8, table_size_L2, shift >= 0 ? shift : 8-table_size_L2);
					vRNG16 *tmp16 = dynamic_cast<vRNG16*>(base_rng);
					if (tmp16) return new BaysDurhamShuffle16(tmp16, table_size_L2, shift >= 0 ? shift : 16-table_size_L2);
					vRNG32 *tmp32 = dynamic_cast<vRNG32*>(base_rng);
					if (tmp32) return new BaysDurhamShuffle32(tmp32, table_size_L2, shift >= 0 ? shift : 32-table_size_L2);
					vRNG64 *tmp64 = dynamic_cast<vRNG64*>(base_rng);
					if (tmp64) return new BaysDurhamShuffle64(tmp64, table_size_L2, shift >= 0 ? shift : 64-table_size_L2);
					issue_error();
					return NULL;//just to quiet the warnings
				}
			}
		}
	}
}
