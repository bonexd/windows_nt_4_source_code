#include "insignia.h"
#include "host_def.h"
#include "ga_mark.h"
#include "cpu_vid.h"
#include "evidfunc.h"


EVID_WRT_POINTERS simple_evid = 
	{
	S_2139_SimpleByteWrite,
	S_2142_SimpleWordWrite,
	S_2145_SimpleDwordWrite,
	S_2140_SimpleByteFill,
	S_2143_SimpleWordFill,
	S_2146_SimpleDwordFill,
	S_2141_SimpleByteMove_Fwd,
	S_2148_SimpleByteMove_Bwd,
	S_2144_SimpleWordMove_Fwd,
	S_2149_SimpleWordMove_Bwd,
	S_2147_SimpleDwordMove_Fwd,
	S_2150_SimpleDwordMove_Bwd
	};

EVID_WRT_POINTERS gricvid_evid = 
	{
	S_2238_GenericByteWrite,
	S_2241_GenericWordWrite,
	S_2244_GenericDwordWrite,
	S_2239_GenericByteFill,
	S_2242_GenericWordFill,
	S_2245_GenericDwordFill,
	S_2240_GenericByteMove_Fwd,
	S_2247_GenericByteMove_Bwd,
	S_2243_GenericWordMove_Fwd,
	S_2248_GenericWordMove_Bwd,
	S_2246_GenericDwordMove_Fwd,
	S_2249_GenericDwordMove_Bwd
	};

EVID_WRT_POINTERS dith_evid[] = {
	{
	S_2151_UnchainedByteWrite_00000000_0000000e_00000001,
	S_2157_UnchainedWordWrite_00000000_0000000e_00000001,
	S_2163_UnchainedDwordWrite_00000000_0000000e_00000001,
	S_2152_UnchainedByteFill_00000000_0000000e_00000001,
	S_2158_UnchainedWordFill_00000000_0000000e_00000001,
	S_2164_UnchainedDwordFill_00000000_0000000e_00000001,
	S_2154_UnchainedByteMove_00000000_0000000e_00000001_00000000,
	S_2205_UnchainedByteMove_00000000_0000000e_00000001_00000001,
	S_2160_UnchainedWordMove_00000000_0000000e_00000001_00000000,
	S_2208_UnchainedWordMove_00000000_0000000e_00000001_00000001,
	S_2165_UnchainedDwordMove_00000000_0000000e_00000001_00000000,
	S_2211_UnchainedDwordMove_00000000_0000000e_00000001_00000001
	},
	{
	S_2166_UnchainedByteWrite_00000001_0000000e_00000001,
	S_2171_UnchainedWordWrite_00000001_0000000e_00000001,
	S_2176_UnchainedDwordWrite_00000001_0000000e_00000001,
	S_2167_UnchainedByteFill_00000001_0000000e_00000001,
	S_2172_UnchainedWordFill_00000001_0000000e_00000001,
	S_2177_UnchainedDwordFill_00000001_0000000e_00000001,
	S_2168_UnchainedByteMove_00000001_0000000e_00000001_00000000,
	S_2212_UnchainedByteMove_00000001_0000000e_00000001_00000001,
	S_2173_UnchainedWordMove_00000001_0000000e_00000001_00000000,
	S_2215_UnchainedWordMove_00000001_0000000e_00000001_00000001,
	S_2178_UnchainedDwordMove_00000001_0000000e_00000001_00000000,
	S_2218_UnchainedDwordMove_00000001_0000000e_00000001_00000001
	},
	{
	S_2179_UnchainedByteWrite_00000002_0000000e_00000001,
	S_2184_UnchainedWordWrite_00000002_0000000e_00000001,
	S_2189_UnchainedDwordWrite_00000002_0000000e_00000001,
	S_2180_UnchainedByteFill_00000002_0000000e_00000001,
	S_2185_UnchainedWordFill_00000002_0000000e_00000001,
	S_2190_UnchainedDwordFill_00000002_0000000e_00000001,
	S_2181_UnchainedByteMove_00000002_0000000e_00000001_00000000,
	S_2219_UnchainedByteMove_00000002_0000000e_00000001_00000001,
	S_2186_UnchainedWordMove_00000002_0000000e_00000001_00000000,
	S_2222_UnchainedWordMove_00000002_0000000e_00000001_00000001,
	S_2191_UnchainedDwordMove_00000002_0000000e_00000001_00000000,
	S_2225_UnchainedDwordMove_00000002_0000000e_00000001_00000001
	},
	{
	S_2192_UnchainedByteWrite_00000003_0000000e_00000001,
	S_2197_UnchainedWordWrite_00000003_0000000e_00000001,
	S_2202_UnchainedDwordWrite_00000003_0000000e_00000001,
	S_2193_UnchainedByteFill_00000003_0000000e_00000001,
	S_2198_UnchainedWordFill_00000003_0000000e_00000001,
	S_2203_UnchainedDwordFill_00000003_0000000e_00000001,
	S_2194_UnchainedByteMove_00000003_0000000e_00000001_00000000,
	S_2226_UnchainedByteMove_00000003_0000000e_00000001_00000001,
	S_2199_UnchainedWordMove_00000003_0000000e_00000001_00000000,
	S_2229_UnchainedWordMove_00000003_0000000e_00000001_00000001,
	S_2204_UnchainedDwordMove_00000003_0000000e_00000001_00000000,
	S_2232_UnchainedDwordMove_00000003_0000000e_00000001_00000001
	}
};

EVID_WRT_POINTERS chain2_evid[] = {
	{
	S_2624_Chain2ByteWrite_00000000,
	S_2630_Chain2WordWrite_00000000,
	S_2636_Chain2DwordWrite_00000000,
	S_2626_Chain2ByteFill_00000000,
	S_2632_Chain2WordFill_00000000,
	S_2638_Chain2DwordFill_00000000,
	S_2628_Chain2ByteMove_00000000_Fwd,
	S_2705_Chain2ByteMove_00000000_Bwd,
	S_2634_Chain2WordMove_00000000_Fwd,
	S_2707_Chain2WordMove_00000000_Bwd,
	S_2640_Chain2DwordMove_00000000_Fwd,
	S_2709_Chain2DwordMove_00000000_Bwd
	},
	{
	S_2642_Chain2ByteWrite_00000001,
	S_2648_Chain2WordWrite_00000001,
	S_2654_Chain2DwordWrite_00000001,
	S_2644_Chain2ByteFill_00000001,
	S_2650_Chain2WordFill_00000001,
	S_2656_Chain2DwordFill_00000001,
	S_2646_Chain2ByteMove_00000001_Fwd,
	S_2711_Chain2ByteMove_00000001_Bwd,
	S_2652_Chain2WordMove_00000001_Fwd,
	S_2713_Chain2WordMove_00000001_Bwd,
	S_2658_Chain2DwordMove_00000001_Fwd,
	S_2715_Chain2DwordMove_00000001_Bwd
	},
	{
	S_2660_Chain2ByteWrite_00000002,
	S_2666_Chain2WordWrite_00000002,
	S_2672_Chain2DwordWrite_00000002,
	S_2662_Chain2ByteFill_00000002,
	S_2668_Chain2WordFill_00000002,
	S_2674_Chain2DwordFill_00000002,
	S_2664_Chain2ByteMove_00000002_Fwd,
	S_2717_Chain2ByteMove_00000002_Bwd,
	S_2670_Chain2WordMove_00000002_Fwd,
	S_2719_Chain2WordMove_00000002_Bwd,
	S_2676_Chain2DwordMove_00000002_Fwd,
	S_2721_Chain2DwordMove_00000002_Bwd
	},
	{
	S_2678_Chain2ByteWrite_00000003,
	S_2684_Chain2WordWrite_00000003,
	S_2690_Chain2DwordWrite_00000003,
	S_2680_Chain2ByteFill_00000003,
	S_2686_Chain2WordFill_00000003,
	S_2692_Chain2DwordFill_00000003,
	S_2682_Chain2ByteMove_00000003_Fwd,
	S_2723_Chain2ByteMove_00000003_Bwd,
	S_2688_Chain2WordMove_00000003_Fwd,
	S_2725_Chain2WordMove_00000003_Bwd,
	S_2694_Chain2DwordMove_00000003_Fwd,
	S_2727_Chain2DwordMove_00000003_Bwd
	},
	{
	S_2696_Chain2ByteWrite_Copy,
	S_2699_Chain2WordWrite_Copy,
	S_2702_Chain2DwordWrite_Copy,
	S_2697_Chain2ByteFill_Copy,
	S_2700_Chain2WordFill_Copy,
	S_2703_Chain2DwordFill_Copy,
	S_2698_Chain2ByteMove_Copy_Fwd,
	S_2729_Chain2ByteMove_Copy_Bwd,
	S_2701_Chain2WordMove_Copy_Fwd,
	S_2730_Chain2WordMove_Copy_Bwd,
	S_2704_Chain2DwordMove_Copy_Fwd,
	S_2731_Chain2DwordMove_Copy_Bwd
	}
};

EVID_WRT_POINTERS unchained_evid[] = {
	{
	S_2250_UnchainedByteWrite_00000000_00000008_00000000,
	S_2305_UnchainedWordWrite_00000000_00000008_00000000,
	S_2360_UnchainedDwordWrite_00000000_00000008_00000000,
	S_2262_UnchainedByteFill_00000000_00000008_00000000,
	S_2317_UnchainedWordFill_00000000_00000008_00000000,
	S_2372_UnchainedDwordFill_00000000_00000008_00000000,
	S_2275_UnchainedByteMove_00000000_00000008_00000000_00000000,
	S_2501_UnchainedByteMove_00000000_00000008_00000000_00000001,
	S_2330_UnchainedWordMove_00000000_00000008_00000000_00000000,
	S_2531_UnchainedWordMove_00000000_00000008_00000000_00000001,
	S_2384_UnchainedDwordMove_00000000_00000008_00000000_00000000,
	S_2561_UnchainedDwordMove_00000000_00000008_00000000_00000001
	},
	{
	S_2251_UnchainedByteWrite_00000000_00000009_00000000,
	S_2306_UnchainedWordWrite_00000000_00000009_00000000,
	S_2361_UnchainedDwordWrite_00000000_00000009_00000000,
	S_2263_UnchainedByteFill_00000000_00000009_00000000,
	S_2318_UnchainedWordFill_00000000_00000009_00000000,
	S_2373_UnchainedDwordFill_00000000_00000009_00000000,
	S_2278_UnchainedByteMove_00000000_00000009_00000000_00000000,
	S_2504_UnchainedByteMove_00000000_00000009_00000000_00000001,
	S_2333_UnchainedWordMove_00000000_00000009_00000000_00000000,
	S_2534_UnchainedWordMove_00000000_00000009_00000000_00000001,
	S_2385_UnchainedDwordMove_00000000_00000009_00000000_00000000,
	S_2562_UnchainedDwordMove_00000000_00000009_00000000_00000001
	},
	{
	S_2252_UnchainedByteWrite_00000000_0000000e_00000000,
	S_2307_UnchainedWordWrite_00000000_0000000e_00000000,
	S_2362_UnchainedDwordWrite_00000000_0000000e_00000000,
	S_2265_UnchainedByteFill_00000000_0000000e_00000000,
	S_2320_UnchainedWordFill_00000000_0000000e_00000000,
	S_2374_UnchainedDwordFill_00000000_0000000e_00000000,
	S_2280_UnchainedByteMove_00000000_0000000e_00000000_00000000,
	S_2506_UnchainedByteMove_00000000_0000000e_00000000_00000001,
	S_2335_UnchainedWordMove_00000000_0000000e_00000000_00000000,
	S_2536_UnchainedWordMove_00000000_0000000e_00000000_00000001,
	S_2386_UnchainedDwordMove_00000000_0000000e_00000000_00000000,
	S_2563_UnchainedDwordMove_00000000_0000000e_00000000_00000001
	},
	{
	S_2253_UnchainedByteWrite_00000000_0000000f_00000000,
	S_2308_UnchainedWordWrite_00000000_0000000f_00000000,
	S_2363_UnchainedDwordWrite_00000000_0000000f_00000000,
	S_2266_UnchainedByteFill_00000000_0000000f_00000000,
	S_2321_UnchainedWordFill_00000000_0000000f_00000000,
	S_2375_UnchainedDwordFill_00000000_0000000f_00000000,
	S_2283_UnchainedByteMove_00000000_0000000f_00000000_00000000,
	S_2509_UnchainedByteMove_00000000_0000000f_00000000_00000001,
	S_2338_UnchainedWordMove_00000000_0000000f_00000000_00000000,
	S_2539_UnchainedWordMove_00000000_0000000f_00000000_00000001,
	S_2387_UnchainedDwordMove_00000000_0000000f_00000000_00000000,
	S_2564_UnchainedDwordMove_00000000_0000000f_00000000_00000001
	},
	{
	S_2254_UnchainedByteWrite_00000000_00000010_00000000,
	S_2309_UnchainedWordWrite_00000000_00000010_00000000,
	S_2364_UnchainedDwordWrite_00000000_00000010_00000000,
	S_2267_UnchainedByteFill_00000000_00000010_00000000,
	S_2322_UnchainedWordFill_00000000_00000010_00000000,
	S_2376_UnchainedDwordFill_00000000_00000010_00000000,
	S_2285_UnchainedByteMove_00000000_00000010_00000000_00000000,
	S_2511_UnchainedByteMove_00000000_00000010_00000000_00000001,
	S_2340_UnchainedWordMove_00000000_00000010_00000000_00000000,
	S_2541_UnchainedWordMove_00000000_00000010_00000000_00000001,
	S_2388_UnchainedDwordMove_00000000_00000010_00000000_00000000,
	S_2565_UnchainedDwordMove_00000000_00000010_00000000_00000001
	},
	{
	S_2255_UnchainedByteWrite_00000000_00000011_00000000,
	S_2310_UnchainedWordWrite_00000000_00000011_00000000,
	S_2365_UnchainedDwordWrite_00000000_00000011_00000000,
	S_2268_UnchainedByteFill_00000000_00000011_00000000,
	S_2323_UnchainedWordFill_00000000_00000011_00000000,
	S_2377_UnchainedDwordFill_00000000_00000011_00000000,
	S_2288_UnchainedByteMove_00000000_00000011_00000000_00000000,
	S_2514_UnchainedByteMove_00000000_00000011_00000000_00000001,
	S_2343_UnchainedWordMove_00000000_00000011_00000000_00000000,
	S_2544_UnchainedWordMove_00000000_00000011_00000000_00000001,
	S_2389_UnchainedDwordMove_00000000_00000011_00000000_00000000,
	S_2566_UnchainedDwordMove_00000000_00000011_00000000_00000001
	},
	{
	S_2256_UnchainedByteWrite_00000000_00000016_00000000,
	S_2311_UnchainedWordWrite_00000000_00000016_00000000,
	S_2366_UnchainedDwordWrite_00000000_00000016_00000000,
	S_2269_UnchainedByteFill_00000000_00000016_00000000,
	S_2324_UnchainedWordFill_00000000_00000016_00000000,
	S_2378_UnchainedDwordFill_00000000_00000016_00000000,
	S_2290_UnchainedByteMove_00000000_00000016_00000000_00000000,
	S_2516_UnchainedByteMove_00000000_00000016_00000000_00000001,
	S_2345_UnchainedWordMove_00000000_00000016_00000000_00000000,
	S_2546_UnchainedWordMove_00000000_00000016_00000000_00000001,
	S_2390_UnchainedDwordMove_00000000_00000016_00000000_00000000,
	S_2567_UnchainedDwordMove_00000000_00000016_00000000_00000001
	},
	{
	S_2257_UnchainedByteWrite_00000000_00000017_00000000,
	S_2312_UnchainedWordWrite_00000000_00000017_00000000,
	S_2367_UnchainedDwordWrite_00000000_00000017_00000000,
	S_2270_UnchainedByteFill_00000000_00000017_00000000,
	S_2325_UnchainedWordFill_00000000_00000017_00000000,
	S_2379_UnchainedDwordFill_00000000_00000017_00000000,
	S_2293_UnchainedByteMove_00000000_00000017_00000000_00000000,
	S_2519_UnchainedByteMove_00000000_00000017_00000000_00000001,
	S_2348_UnchainedWordMove_00000000_00000017_00000000_00000000,
	S_2549_UnchainedWordMove_00000000_00000017_00000000_00000001,
	S_2391_UnchainedDwordMove_00000000_00000017_00000000_00000000,
	S_2568_UnchainedDwordMove_00000000_00000017_00000000_00000001
	},
	{
	S_2258_UnchainedByteWrite_00000000_00000018_00000000,
	S_2313_UnchainedWordWrite_00000000_00000018_00000000,
	S_2368_UnchainedDwordWrite_00000000_00000018_00000000,
	S_2271_UnchainedByteFill_00000000_00000018_00000000,
	S_2326_UnchainedWordFill_00000000_00000018_00000000,
	S_2380_UnchainedDwordFill_00000000_00000018_00000000,
	S_2295_UnchainedByteMove_00000000_00000018_00000000_00000000,
	S_2521_UnchainedByteMove_00000000_00000018_00000000_00000001,
	S_2350_UnchainedWordMove_00000000_00000018_00000000_00000000,
	S_2551_UnchainedWordMove_00000000_00000018_00000000_00000001,
	S_2392_UnchainedDwordMove_00000000_00000018_00000000_00000000,
	S_2569_UnchainedDwordMove_00000000_00000018_00000000_00000001
	},
	{
	S_2259_UnchainedByteWrite_00000000_00000019_00000000,
	S_2314_UnchainedWordWrite_00000000_00000019_00000000,
	S_2369_UnchainedDwordWrite_00000000_00000019_00000000,
	S_2272_UnchainedByteFill_00000000_00000019_00000000,
	S_2327_UnchainedWordFill_00000000_00000019_00000000,
	S_2381_UnchainedDwordFill_00000000_00000019_00000000,
	S_2298_UnchainedByteMove_00000000_00000019_00000000_00000000,
	S_2524_UnchainedByteMove_00000000_00000019_00000000_00000001,
	S_2353_UnchainedWordMove_00000000_00000019_00000000_00000000,
	S_2554_UnchainedWordMove_00000000_00000019_00000000_00000001,
	S_2393_UnchainedDwordMove_00000000_00000019_00000000_00000000,
	S_2570_UnchainedDwordMove_00000000_00000019_00000000_00000001
	},
	{
	S_2260_UnchainedByteWrite_00000000_0000001e_00000000,
	S_2315_UnchainedWordWrite_00000000_0000001e_00000000,
	S_2370_UnchainedDwordWrite_00000000_0000001e_00000000,
	S_2273_UnchainedByteFill_00000000_0000001e_00000000,
	S_2328_UnchainedWordFill_00000000_0000001e_00000000,
	S_2382_UnchainedDwordFill_00000000_0000001e_00000000,
	S_2300_UnchainedByteMove_00000000_0000001e_00000000_00000000,
	S_2526_UnchainedByteMove_00000000_0000001e_00000000_00000001,
	S_2355_UnchainedWordMove_00000000_0000001e_00000000_00000000,
	S_2556_UnchainedWordMove_00000000_0000001e_00000000_00000001,
	S_2394_UnchainedDwordMove_00000000_0000001e_00000000_00000000,
	S_2571_UnchainedDwordMove_00000000_0000001e_00000000_00000001
	},
	{
	S_2261_UnchainedByteWrite_00000000_0000001f_00000000,
	S_2316_UnchainedWordWrite_00000000_0000001f_00000000,
	S_2371_UnchainedDwordWrite_00000000_0000001f_00000000,
	S_2274_UnchainedByteFill_00000000_0000001f_00000000,
	S_2329_UnchainedWordFill_00000000_0000001f_00000000,
	S_2383_UnchainedDwordFill_00000000_0000001f_00000000,
	S_2303_UnchainedByteMove_00000000_0000001f_00000000_00000000,
	S_2529_UnchainedByteMove_00000000_0000001f_00000000_00000001,
	S_2358_UnchainedWordMove_00000000_0000001f_00000000_00000000,
	S_2559_UnchainedWordMove_00000000_0000001f_00000000_00000001,
	S_2395_UnchainedDwordMove_00000000_0000001f_00000000_00000000,
	S_2572_UnchainedDwordMove_00000000_0000001f_00000000_00000001
	},
	{
	S_2396_UnchainedByteWrite_00000001_00000000_00000000,
	S_2399_UnchainedWordWrite_00000001_00000000_00000000,
	S_2402_UnchainedDwordWrite_00000001_00000000_00000000,
	S_2397_UnchainedByteFill_00000001_00000000_00000000,
	S_2400_UnchainedWordFill_00000001_00000000_00000000,
	S_2403_UnchainedDwordFill_00000001_00000000_00000000,
	S_2398_UnchainedByteMove_00000001_00000000_00000000_00000000,
	S_2573_UnchainedByteMove_00000001_00000000_00000000_00000001,
	S_2401_UnchainedWordMove_00000001_00000000_00000000_00000000,
	S_2574_UnchainedWordMove_00000001_00000000_00000000_00000001,
	S_2404_UnchainedDwordMove_00000001_00000000_00000000_00000000,
	S_2575_UnchainedDwordMove_00000001_00000000_00000000_00000001
	},
	{
	S_2405_UnchainedByteWrite_00000002_00000008_00000000,
	S_2423_UnchainedWordWrite_00000002_00000008_00000000,
	S_2441_UnchainedDwordWrite_00000002_00000008_00000000,
	S_2409_UnchainedByteFill_00000002_00000008_00000000,
	S_2427_UnchainedWordFill_00000002_00000008_00000000,
	S_2445_UnchainedDwordFill_00000002_00000008_00000000,
	S_2413_UnchainedByteMove_00000002_00000008_00000000_00000000,
	S_2576_UnchainedByteMove_00000002_00000008_00000000_00000001,
	S_2431_UnchainedWordMove_00000002_00000008_00000000_00000000,
	S_2586_UnchainedWordMove_00000002_00000008_00000000_00000001,
	S_2449_UnchainedDwordMove_00000002_00000008_00000000_00000000,
	S_2596_UnchainedDwordMove_00000002_00000008_00000000_00000001
	},
	{
	S_2406_UnchainedByteWrite_00000002_00000009_00000000,
	S_2424_UnchainedWordWrite_00000002_00000009_00000000,
	S_2442_UnchainedDwordWrite_00000002_00000009_00000000,
	S_2410_UnchainedByteFill_00000002_00000009_00000000,
	S_2428_UnchainedWordFill_00000002_00000009_00000000,
	S_2446_UnchainedDwordFill_00000002_00000009_00000000,
	S_2416_UnchainedByteMove_00000002_00000009_00000000_00000000,
	S_2579_UnchainedByteMove_00000002_00000009_00000000_00000001,
	S_2434_UnchainedWordMove_00000002_00000009_00000000_00000000,
	S_2589_UnchainedWordMove_00000002_00000009_00000000_00000001,
	S_2450_UnchainedDwordMove_00000002_00000009_00000000_00000000,
	S_2597_UnchainedDwordMove_00000002_00000009_00000000_00000001
	},
	{
	S_2407_UnchainedByteWrite_00000002_0000000e_00000000,
	S_2425_UnchainedWordWrite_00000002_0000000e_00000000,
	S_2443_UnchainedDwordWrite_00000002_0000000e_00000000,
	S_2411_UnchainedByteFill_00000002_0000000e_00000000,
	S_2429_UnchainedWordFill_00000002_0000000e_00000000,
	S_2447_UnchainedDwordFill_00000002_0000000e_00000000,
	S_2418_UnchainedByteMove_00000002_0000000e_00000000_00000000,
	S_2581_UnchainedByteMove_00000002_0000000e_00000000_00000001,
	S_2436_UnchainedWordMove_00000002_0000000e_00000000_00000000,
	S_2591_UnchainedWordMove_00000002_0000000e_00000000_00000001,
	S_2451_UnchainedDwordMove_00000002_0000000e_00000000_00000000,
	S_2598_UnchainedDwordMove_00000002_0000000e_00000000_00000001
	},
	{
	S_2408_UnchainedByteWrite_00000002_0000000f_00000000,
	S_2426_UnchainedWordWrite_00000002_0000000f_00000000,
	S_2444_UnchainedDwordWrite_00000002_0000000f_00000000,
	S_2412_UnchainedByteFill_00000002_0000000f_00000000,
	S_2430_UnchainedWordFill_00000002_0000000f_00000000,
	S_2448_UnchainedDwordFill_00000002_0000000f_00000000,
	S_2421_UnchainedByteMove_00000002_0000000f_00000000_00000000,
	S_2584_UnchainedByteMove_00000002_0000000f_00000000_00000001,
	S_2439_UnchainedWordMove_00000002_0000000f_00000000_00000000,
	S_2594_UnchainedWordMove_00000002_0000000f_00000000_00000001,
	S_2452_UnchainedDwordMove_00000002_0000000f_00000000_00000000,
	S_2599_UnchainedDwordMove_00000002_0000000f_00000000_00000001
	},
	{
	S_2453_UnchainedByteWrite_00000003_00000008_00000000,
	S_2471_UnchainedWordWrite_00000003_00000008_00000000,
	S_2489_UnchainedDwordWrite_00000003_00000008_00000000,
	S_2457_UnchainedByteFill_00000003_00000008_00000000,
	S_2475_UnchainedWordFill_00000003_00000008_00000000,
	S_2493_UnchainedDwordFill_00000003_00000008_00000000,
	S_2461_UnchainedByteMove_00000003_00000008_00000000_00000000,
	S_2600_UnchainedByteMove_00000003_00000008_00000000_00000001,
	S_2479_UnchainedWordMove_00000003_00000008_00000000_00000000,
	S_2610_UnchainedWordMove_00000003_00000008_00000000_00000001,
	S_2497_UnchainedDwordMove_00000003_00000008_00000000_00000000,
	S_2620_UnchainedDwordMove_00000003_00000008_00000000_00000001
	},
	{
	S_2454_UnchainedByteWrite_00000003_00000009_00000000,
	S_2472_UnchainedWordWrite_00000003_00000009_00000000,
	S_2490_UnchainedDwordWrite_00000003_00000009_00000000,
	S_2458_UnchainedByteFill_00000003_00000009_00000000,
	S_2476_UnchainedWordFill_00000003_00000009_00000000,
	S_2494_UnchainedDwordFill_00000003_00000009_00000000,
	S_2464_UnchainedByteMove_00000003_00000009_00000000_00000000,
	S_2603_UnchainedByteMove_00000003_00000009_00000000_00000001,
	S_2482_UnchainedWordMove_00000003_00000009_00000000_00000000,
	S_2613_UnchainedWordMove_00000003_00000009_00000000_00000001,
	S_2498_UnchainedDwordMove_00000003_00000009_00000000_00000000,
	S_2621_UnchainedDwordMove_00000003_00000009_00000000_00000001
	},
	{
	S_2455_UnchainedByteWrite_00000003_0000000e_00000000,
	S_2473_UnchainedWordWrite_00000003_0000000e_00000000,
	S_2491_UnchainedDwordWrite_00000003_0000000e_00000000,
	S_2459_UnchainedByteFill_00000003_0000000e_00000000,
	S_2477_UnchainedWordFill_00000003_0000000e_00000000,
	S_2495_UnchainedDwordFill_00000003_0000000e_00000000,
	S_2466_UnchainedByteMove_00000003_0000000e_00000000_00000000,
	S_2605_UnchainedByteMove_00000003_0000000e_00000000_00000001,
	S_2484_UnchainedWordMove_00000003_0000000e_00000000_00000000,
	S_2615_UnchainedWordMove_00000003_0000000e_00000000_00000001,
	S_2499_UnchainedDwordMove_00000003_0000000e_00000000_00000000,
	S_2622_UnchainedDwordMove_00000003_0000000e_00000000_00000001
	},
	{
	S_2456_UnchainedByteWrite_00000003_0000000f_00000000,
	S_2474_UnchainedWordWrite_00000003_0000000f_00000000,
	S_2492_UnchainedDwordWrite_00000003_0000000f_00000000,
	S_2460_UnchainedByteFill_00000003_0000000f_00000000,
	S_2478_UnchainedWordFill_00000003_0000000f_00000000,
	S_2496_UnchainedDwordFill_00000003_0000000f_00000000,
	S_2469_UnchainedByteMove_00000003_0000000f_00000000_00000000,
	S_2608_UnchainedByteMove_00000003_0000000f_00000000_00000001,
	S_2487_UnchainedWordMove_00000003_0000000f_00000000_00000000,
	S_2618_UnchainedWordMove_00000003_0000000f_00000000_00000001,
	S_2500_UnchainedDwordMove_00000003_0000000f_00000000_00000000,
	S_2623_UnchainedDwordMove_00000003_0000000f_00000000_00000001
	}
};

EVID_WRT_POINTERS chain4_evid[] = {
	{
	S_2732_Chain4ByteWrite_00000000_00000008,
	S_2782_Chain4WordWrite_00000000_00000008,
	S_2831_Chain4DwordWrite_00000000_00000008,
	S_2744_Chain4ByteFill_00000000_00000008,
	S_2794_Chain4WordFill_00000000_00000008,
	S_2843_Chain4DwordFill_00000000_00000008,
	S_2758_Chain4ByteMove_00000000_00000008_00000000,
	S_2965_Chain4ByteMove_00000000_00000008_00000001,
	S_2807_Chain4WordMove_00000000_00000008_00000000,
	S_2989_Chain4WordMove_00000000_00000008_00000001,
	S_2855_Chain4DwordMove_00000000_00000008_00000000,
	S_3013_Chain4DwordMove_00000000_00000008_00000001
	},
	{
	S_2733_Chain4ByteWrite_00000000_00000009,
	S_2783_Chain4WordWrite_00000000_00000009,
	S_2832_Chain4DwordWrite_00000000_00000009,
	S_2746_Chain4ByteFill_00000000_00000009,
	S_2795_Chain4WordFill_00000000_00000009,
	S_2844_Chain4DwordFill_00000000_00000009,
	S_2760_Chain4ByteMove_00000000_00000009_00000000,
	S_2967_Chain4ByteMove_00000000_00000009_00000001,
	S_2809_Chain4WordMove_00000000_00000009_00000000,
	S_2991_Chain4WordMove_00000000_00000009_00000001,
	S_2856_Chain4DwordMove_00000000_00000009_00000000,
	S_3014_Chain4DwordMove_00000000_00000009_00000001
	},
	{
	S_2734_Chain4ByteWrite_00000000_0000000e,
	S_2784_Chain4WordWrite_00000000_0000000e,
	S_2833_Chain4DwordWrite_00000000_0000000e,
	S_2748_Chain4ByteFill_00000000_0000000e,
	S_2797_Chain4WordFill_00000000_0000000e,
	S_2845_Chain4DwordFill_00000000_0000000e,
	S_2762_Chain4ByteMove_00000000_0000000e_00000000,
	S_2969_Chain4ByteMove_00000000_0000000e_00000001,
	S_2811_Chain4WordMove_00000000_0000000e_00000000,
	S_2993_Chain4WordMove_00000000_0000000e_00000001,
	S_2857_Chain4DwordMove_00000000_0000000e_00000000,
	S_3015_Chain4DwordMove_00000000_0000000e_00000001
	},
	{
	S_2735_Chain4ByteWrite_00000000_0000000f,
	S_2785_Chain4WordWrite_00000000_0000000f,
	S_2834_Chain4DwordWrite_00000000_0000000f,
	S_2749_Chain4ByteFill_00000000_0000000f,
	S_2798_Chain4WordFill_00000000_0000000f,
	S_2846_Chain4DwordFill_00000000_0000000f,
	S_2764_Chain4ByteMove_00000000_0000000f_00000000,
	S_2971_Chain4ByteMove_00000000_0000000f_00000001,
	S_2813_Chain4WordMove_00000000_0000000f_00000000,
	S_2995_Chain4WordMove_00000000_0000000f_00000001,
	S_2858_Chain4DwordMove_00000000_0000000f_00000000,
	S_3016_Chain4DwordMove_00000000_0000000f_00000001
	},
	{
	S_2736_Chain4ByteWrite_00000000_00000010,
	S_2786_Chain4WordWrite_00000000_00000010,
	S_2835_Chain4DwordWrite_00000000_00000010,
	S_2750_Chain4ByteFill_00000000_00000010,
	S_2799_Chain4WordFill_00000000_00000010,
	S_2847_Chain4DwordFill_00000000_00000010,
	S_2766_Chain4ByteMove_00000000_00000010_00000000,
	S_2973_Chain4ByteMove_00000000_00000010_00000001,
	S_2815_Chain4WordMove_00000000_00000010_00000000,
	S_2997_Chain4WordMove_00000000_00000010_00000001,
	S_2859_Chain4DwordMove_00000000_00000010_00000000,
	S_3017_Chain4DwordMove_00000000_00000010_00000001
	},
	{
	S_2737_Chain4ByteWrite_00000000_00000011,
	S_2787_Chain4WordWrite_00000000_00000011,
	S_2836_Chain4DwordWrite_00000000_00000011,
	S_2751_Chain4ByteFill_00000000_00000011,
	S_2800_Chain4WordFill_00000000_00000011,
	S_2848_Chain4DwordFill_00000000_00000011,
	S_2768_Chain4ByteMove_00000000_00000011_00000000,
	S_2975_Chain4ByteMove_00000000_00000011_00000001,
	S_2817_Chain4WordMove_00000000_00000011_00000000,
	S_2999_Chain4WordMove_00000000_00000011_00000001,
	S_2860_Chain4DwordMove_00000000_00000011_00000000,
	S_3018_Chain4DwordMove_00000000_00000011_00000001
	},
	{
	S_2738_Chain4ByteWrite_00000000_00000016,
	S_2788_Chain4WordWrite_00000000_00000016,
	S_2837_Chain4DwordWrite_00000000_00000016,
	S_2752_Chain4ByteFill_00000000_00000016,
	S_2801_Chain4WordFill_00000000_00000016,
	S_2849_Chain4DwordFill_00000000_00000016,
	S_2770_Chain4ByteMove_00000000_00000016_00000000,
	S_2977_Chain4ByteMove_00000000_00000016_00000001,
	S_2819_Chain4WordMove_00000000_00000016_00000000,
	S_3001_Chain4WordMove_00000000_00000016_00000001,
	S_2861_Chain4DwordMove_00000000_00000016_00000000,
	S_3019_Chain4DwordMove_00000000_00000016_00000001
	},
	{
	S_2739_Chain4ByteWrite_00000000_00000017,
	S_2789_Chain4WordWrite_00000000_00000017,
	S_2838_Chain4DwordWrite_00000000_00000017,
	S_2753_Chain4ByteFill_00000000_00000017,
	S_2802_Chain4WordFill_00000000_00000017,
	S_2850_Chain4DwordFill_00000000_00000017,
	S_2772_Chain4ByteMove_00000000_00000017_00000000,
	S_2979_Chain4ByteMove_00000000_00000017_00000001,
	S_2821_Chain4WordMove_00000000_00000017_00000000,
	S_3003_Chain4WordMove_00000000_00000017_00000001,
	S_2862_Chain4DwordMove_00000000_00000017_00000000,
	S_3020_Chain4DwordMove_00000000_00000017_00000001
	},
	{
	S_2740_Chain4ByteWrite_00000000_00000018,
	S_2790_Chain4WordWrite_00000000_00000018,
	S_2839_Chain4DwordWrite_00000000_00000018,
	S_2754_Chain4ByteFill_00000000_00000018,
	S_2803_Chain4WordFill_00000000_00000018,
	S_2851_Chain4DwordFill_00000000_00000018,
	S_2774_Chain4ByteMove_00000000_00000018_00000000,
	S_2981_Chain4ByteMove_00000000_00000018_00000001,
	S_2823_Chain4WordMove_00000000_00000018_00000000,
	S_3005_Chain4WordMove_00000000_00000018_00000001,
	S_2863_Chain4DwordMove_00000000_00000018_00000000,
	S_3021_Chain4DwordMove_00000000_00000018_00000001
	},
	{
	S_2741_Chain4ByteWrite_00000000_00000019,
	S_2791_Chain4WordWrite_00000000_00000019,
	S_2840_Chain4DwordWrite_00000000_00000019,
	S_2755_Chain4ByteFill_00000000_00000019,
	S_2804_Chain4WordFill_00000000_00000019,
	S_2852_Chain4DwordFill_00000000_00000019,
	S_2776_Chain4ByteMove_00000000_00000019_00000000,
	S_2983_Chain4ByteMove_00000000_00000019_00000001,
	S_2825_Chain4WordMove_00000000_00000019_00000000,
	S_3007_Chain4WordMove_00000000_00000019_00000001,
	S_2864_Chain4DwordMove_00000000_00000019_00000000,
	S_3022_Chain4DwordMove_00000000_00000019_00000001
	},
	{
	S_2742_Chain4ByteWrite_00000000_0000001e,
	S_2792_Chain4WordWrite_00000000_0000001e,
	S_2841_Chain4DwordWrite_00000000_0000001e,
	S_2756_Chain4ByteFill_00000000_0000001e,
	S_2805_Chain4WordFill_00000000_0000001e,
	S_2853_Chain4DwordFill_00000000_0000001e,
	S_2778_Chain4ByteMove_00000000_0000001e_00000000,
	S_2985_Chain4ByteMove_00000000_0000001e_00000001,
	S_2827_Chain4WordMove_00000000_0000001e_00000000,
	S_3009_Chain4WordMove_00000000_0000001e_00000001,
	S_2865_Chain4DwordMove_00000000_0000001e_00000000,
	S_3023_Chain4DwordMove_00000000_0000001e_00000001
	},
	{
	S_2743_Chain4ByteWrite_00000000_0000001f,
	S_2793_Chain4WordWrite_00000000_0000001f,
	S_2842_Chain4DwordWrite_00000000_0000001f,
	S_2757_Chain4ByteFill_00000000_0000001f,
	S_2806_Chain4WordFill_00000000_0000001f,
	S_2854_Chain4DwordFill_00000000_0000001f,
	S_2780_Chain4ByteMove_00000000_0000001f_00000000,
	S_2987_Chain4ByteMove_00000000_0000001f_00000001,
	S_2829_Chain4WordMove_00000000_0000001f_00000000,
	S_3011_Chain4WordMove_00000000_0000001f_00000001,
	S_2866_Chain4DwordMove_00000000_0000001f_00000000,
	S_3024_Chain4DwordMove_00000000_0000001f_00000001
	},
	{
	S_2867_Chain4ByteWrite_00000001_00000000,
	S_2871_Chain4WordWrite_00000001_00000000,
	S_2874_Chain4DwordWrite_00000001_00000000,
	S_2868_Chain4ByteFill_00000001_00000000,
	S_2872_Chain4WordFill_00000001_00000000,
	S_2875_Chain4DwordFill_00000001_00000000,
	S_2869_Chain4ByteMove_00000001_00000000_00000000,
	S_3025_Chain4ByteMove_00000001_00000000_00000001,
	S_2873_Chain4WordMove_00000001_00000000_00000000,
	S_3027_Chain4WordMove_00000001_00000000_00000001,
	S_2876_Chain4DwordMove_00000001_00000000_00000000,
	S_3028_Chain4DwordMove_00000001_00000000_00000001
	},
	{
	S_2877_Chain4ByteWrite_00000002_00000008,
	S_2893_Chain4WordWrite_00000002_00000008,
	S_2909_Chain4DwordWrite_00000002_00000008,
	S_2881_Chain4ByteFill_00000002_00000008,
	S_2897_Chain4WordFill_00000002_00000008,
	S_2913_Chain4DwordFill_00000002_00000008,
	S_2885_Chain4ByteMove_00000002_00000008_00000000,
	S_3029_Chain4ByteMove_00000002_00000008_00000001,
	S_2901_Chain4WordMove_00000002_00000008_00000000,
	S_3037_Chain4WordMove_00000002_00000008_00000001,
	S_2917_Chain4DwordMove_00000002_00000008_00000000,
	S_3045_Chain4DwordMove_00000002_00000008_00000001
	},
	{
	S_2878_Chain4ByteWrite_00000002_00000009,
	S_2894_Chain4WordWrite_00000002_00000009,
	S_2910_Chain4DwordWrite_00000002_00000009,
	S_2882_Chain4ByteFill_00000002_00000009,
	S_2898_Chain4WordFill_00000002_00000009,
	S_2914_Chain4DwordFill_00000002_00000009,
	S_2887_Chain4ByteMove_00000002_00000009_00000000,
	S_3031_Chain4ByteMove_00000002_00000009_00000001,
	S_2903_Chain4WordMove_00000002_00000009_00000000,
	S_3039_Chain4WordMove_00000002_00000009_00000001,
	S_2918_Chain4DwordMove_00000002_00000009_00000000,
	S_3046_Chain4DwordMove_00000002_00000009_00000001
	},
	{
	S_2879_Chain4ByteWrite_00000002_0000000e,
	S_2895_Chain4WordWrite_00000002_0000000e,
	S_2911_Chain4DwordWrite_00000002_0000000e,
	S_2883_Chain4ByteFill_00000002_0000000e,
	S_2899_Chain4WordFill_00000002_0000000e,
	S_2915_Chain4DwordFill_00000002_0000000e,
	S_2889_Chain4ByteMove_00000002_0000000e_00000000,
	S_3033_Chain4ByteMove_00000002_0000000e_00000001,
	S_2905_Chain4WordMove_00000002_0000000e_00000000,
	S_3041_Chain4WordMove_00000002_0000000e_00000001,
	S_2919_Chain4DwordMove_00000002_0000000e_00000000,
	S_3047_Chain4DwordMove_00000002_0000000e_00000001
	},
	{
	S_2880_Chain4ByteWrite_00000002_0000000f,
	S_2896_Chain4WordWrite_00000002_0000000f,
	S_2912_Chain4DwordWrite_00000002_0000000f,
	S_2884_Chain4ByteFill_00000002_0000000f,
	S_2900_Chain4WordFill_00000002_0000000f,
	S_2916_Chain4DwordFill_00000002_0000000f,
	S_2891_Chain4ByteMove_00000002_0000000f_00000000,
	S_3035_Chain4ByteMove_00000002_0000000f_00000001,
	S_2907_Chain4WordMove_00000002_0000000f_00000000,
	S_3043_Chain4WordMove_00000002_0000000f_00000001,
	S_2920_Chain4DwordMove_00000002_0000000f_00000000,
	S_3048_Chain4DwordMove_00000002_0000000f_00000001
	},
	{
	S_2921_Chain4ByteWrite_00000003_00000008,
	S_2937_Chain4WordWrite_00000003_00000008,
	S_2953_Chain4DwordWrite_00000003_00000008,
	S_2925_Chain4ByteFill_00000003_00000008,
	S_2941_Chain4WordFill_00000003_00000008,
	S_2957_Chain4DwordFill_00000003_00000008,
	S_2929_Chain4ByteMove_00000003_00000008_00000000,
	S_3049_Chain4ByteMove_00000003_00000008_00000001,
	S_2945_Chain4WordMove_00000003_00000008_00000000,
	S_3057_Chain4WordMove_00000003_00000008_00000001,
	S_2961_Chain4DwordMove_00000003_00000008_00000000,
	S_3065_Chain4DwordMove_00000003_00000008_00000001
	},
	{
	S_2922_Chain4ByteWrite_00000003_00000009,
	S_2938_Chain4WordWrite_00000003_00000009,
	S_2954_Chain4DwordWrite_00000003_00000009,
	S_2926_Chain4ByteFill_00000003_00000009,
	S_2942_Chain4WordFill_00000003_00000009,
	S_2958_Chain4DwordFill_00000003_00000009,
	S_2931_Chain4ByteMove_00000003_00000009_00000000,
	S_3051_Chain4ByteMove_00000003_00000009_00000001,
	S_2947_Chain4WordMove_00000003_00000009_00000000,
	S_3059_Chain4WordMove_00000003_00000009_00000001,
	S_2962_Chain4DwordMove_00000003_00000009_00000000,
	S_3066_Chain4DwordMove_00000003_00000009_00000001
	},
	{
	S_2923_Chain4ByteWrite_00000003_0000000e,
	S_2939_Chain4WordWrite_00000003_0000000e,
	S_2955_Chain4DwordWrite_00000003_0000000e,
	S_2927_Chain4ByteFill_00000003_0000000e,
	S_2943_Chain4WordFill_00000003_0000000e,
	S_2959_Chain4DwordFill_00000003_0000000e,
	S_2933_Chain4ByteMove_00000003_0000000e_00000000,
	S_3053_Chain4ByteMove_00000003_0000000e_00000001,
	S_2949_Chain4WordMove_00000003_0000000e_00000000,
	S_3061_Chain4WordMove_00000003_0000000e_00000001,
	S_2963_Chain4DwordMove_00000003_0000000e_00000000,
	S_3067_Chain4DwordMove_00000003_0000000e_00000001
	},
	{
	S_2924_Chain4ByteWrite_00000003_0000000f,
	S_2940_Chain4WordWrite_00000003_0000000f,
	S_2956_Chain4DwordWrite_00000003_0000000f,
	S_2928_Chain4ByteFill_00000003_0000000f,
	S_2944_Chain4WordFill_00000003_0000000f,
	S_2960_Chain4DwordFill_00000003_0000000f,
	S_2935_Chain4ByteMove_00000003_0000000f_00000000,
	S_3055_Chain4ByteMove_00000003_0000000f_00000001,
	S_2951_Chain4WordMove_00000003_0000000f_00000000,
	S_3063_Chain4WordMove_00000003_0000000f_00000001,
	S_2964_Chain4DwordMove_00000003_0000000f_00000000,
	S_3068_Chain4DwordMove_00000003_0000000f_00000001
	}
};


EVID_READ_POINTERS simple_read_evid = 
	{
	S_2115_SimpleByteRead,
	S_2116_SimpleWordRead,
	S_2117_SimpleDwordRead,
	S_2118_SimpleStringRead,
	S_2118_SimpleStringRead
	};

EVID_READ_POINTERS ram_dsbld_read_evid = 
	{
	S_2111_DisabledRAMByteRead,
	S_2112_DisabledRAMWordRead,
	S_2113_DisabledRAMDwordRead,
	S_2114_DisabledRAMStringReadFwd,
	S_2125_DisabledRAMStringReadBwd
	};

EVID_READ_POINTERS read_mode0_evid[] = {
	{
	S_2095_RdMode0UnchainedByteRead,
	S_2096_RdMode0UnchainedWordRead,
	S_2097_RdMode0UnchainedDwordRead,
	S_2098_RdMode0UnchainedStringReadFwd,
	S_2121_RdMode0UnchainedStringReadBwd
	},
	{
	S_2087_RdMode0Chain2ByteRead,
	S_2088_RdMode0Chain2WordRead,
	S_2089_RdMode0Chain2DwordRead,
	S_2090_RdMode0Chain2StringReadFwd,
	S_2119_RdMode0Chain2StringReadBwd
	},
	{
	S_2091_RdMode0Chain4ByteRead,
	S_2092_RdMode0Chain4WordRead,
	S_2093_RdMode0Chain4DwordRead,
	S_2094_RdMode0Chain4StringReadFwd,
	S_2120_RdMode0Chain4StringReadBwd
	}
};

EVID_READ_POINTERS read_mode1_evid[] = {
	{
	S_2107_RdMode1UnchainedByteRead,
	S_2108_RdMode1UnchainedWordRead,
	S_2109_RdMode1UnchainedDwordRead,
	S_2110_RdMode1UnchainedStringReadFwd,
	S_2124_RdMode1UnchainedStringReadBwd
	},
	{
	S_2099_RdMode1Chain2ByteRead,
	S_2100_RdMode1Chain2WordRead,
	S_2101_RdMode1Chain2DwordRead,
	S_2102_RdMode1Chain2StringReadFwd,
	S_2122_RdMode1Chain2StringReadBwd
	},
	{
	S_2103_RdMode1Chain4ByteRead,
	S_2104_RdMode1Chain4WordRead,
	S_2105_RdMode1Chain4DwordRead,
	S_2106_RdMode1Chain4StringReadFwd,
	S_2123_RdMode1Chain4StringReadBwd
	}
};

EVID_MARK_POINTERS simple_mark_evid = 
	{
	S_2126_SimpleMark,
	S_2126_SimpleMark,
	S_2126_SimpleMark,
	S_2126_SimpleMark
	};

EVID_MARK_POINTERS cga_mark_evid = 
	{
	S_2127_CGAMarkByte,
	S_2128_CGAMarkWord,
	S_2129_CGAMarkDword,
	S_2130_CGAMarkString
	};

EVID_MARK_POINTERS unchained_mark_evid = 
	{
	S_2131_UnchainedMarkByte,
	S_2132_UnchainedMarkWord,
	S_2133_UnchainedMarkDword,
	S_2134_UnchainedMarkString
	};

EVID_MARK_POINTERS chain4_mark_evid = 
	{
	S_2135_Chain4MarkByte,
	S_2136_Chain4MarkWord,
	S_2137_Chain4MarkDword,
	S_2138_Chain4MarkString
	};
