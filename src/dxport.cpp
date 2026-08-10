#include <xmmintrin.h>
#include <cstdint>
#include "dxport.hpp"
#include "mem.hpp"
#include "util.hpp"

namespace DXPORT {

	struct Vec3 { float x, y, z; };
	static constexpr float FLT_CHECK = 1.4210855e-14;
	static constexpr float THREE = 3.0f;
	static constexpr float ONEHALF = 1.5f;
	static constexpr const float HALF = 0.5f;

	Vec3* Vec3NormalizeBranch(Vec3* out, const Vec3* in) {
		const float x = in->x;
		const float y = in->y;
		const float z = in->z;
		
		const float length_sq = x * x + y * y + z * z;
		
		if (unlikely(length_sq < FLT_CHECK)) {
			out->x = 0.0f;
			out->y = 0.0f;
			out->z = 0.0f;
			return out;
		}
		
		const float rsqrt_approx = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(length_sq)));
		
		const float rsqrt_refined = rsqrt_approx * (THREE * HALF - HALF * length_sq * rsqrt_approx * rsqrt_approx);
		
		out->x = x * rsqrt_refined;
		out->y = y * rsqrt_refined;
		out->z = z * rsqrt_refined;
		
		return out;
	}
	
	// requires AVX + FMA
	Vec3* Vec3NormalizeBranchless(Vec3* out, const Vec3* in) {
    		const float x = in->x;
    		const float y = in->y;
    		const float z = in->z;
    		
    		const float length_sq = x * x + y * y + z * z;
    		
    		const float rsqrt_approx = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(length_sq)));
    		
    		const float rsqrt_refined_unmasked = rsqrt_approx * (ONEHALF - HALF * length_sq * rsqrt_approx * rsqrt_approx);
    		
    		const float mask = (length_sq < FLT_CHECK) ? 1.0f : 0.0f;
    
    		const float rsqrt_refined = rsqrt_refined_unmasked * (1.0f - mask);
    
    		out->x = x * rsqrt_refined;
    		out->y = y * rsqrt_refined;
    		out->z = z * rsqrt_refined;
    		
    		return out;
    }
    
	void Initialize() {
		uint64_t pVec3 = 0ull;
		// outdated
		const std::string_view Vec3Byte = "ff 15 ?? ?? ?? ?? 41 ff c7 48 ff c5";
		Memory::PatternScan(Vec3Byte, pVec3);
		pVec3 = Memory::moduleBase + Memory::FindRVA(Memory::moduleBase, std::bit_cast<void*>(pVec3), 2, 6);
		Memory::IatMod(std::bit_cast<uint64_t*>(pVec3), Vec3NormalizeBranch);
	}
	
}