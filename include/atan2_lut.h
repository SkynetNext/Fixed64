#pragma once

#include <array>
#include <cstdint>

namespace math::fp::detail {

// Arctangent lookup table with 256 entries for atan2 implementation
// Input range: [0, 1], Output: atan(x) in radians [0, pi/4]
// Fixed-point format: Q31.32
inline constexpr std::array<int64_t, 256> kAtan2LUT = {
    0LL, 16842922LL, 33685327LL, 50526695LL, 67366510LL, 84204254LL, 
    101039410LL, 117871461LL, 134699891LL, 151524185LL, 168343828LL, 185158307LL, 
    201967108LL, 218769720LL, 235565633LL, 252354337LL, 269135323LL, 285908086LL, 
    302672120LL, 319426921LL, 336171988LL, 352906821LL, 369630921LL, 386343791LL, 
    403044939LL, 419733871LL, 436410097LL, 453073130LL, 469722484LL, 486357677LL, 
    502978227LL, 519583659LL, 536173495LL, 552747264LL, 569304496LL, 585844724LL, 
    602367486LL, 618872320LL, 635358769LL, 651826378LL, 668274697LL, 684703278LL, 
    701111675LL, 717499450LL, 733866163LL, 750211382LL, 766534675LL, 782835618LL, 
    799113786LL, 815368760LL, 831600127LL, 847807473LL, 863990393LL, 880148483LL, 
    896281344LL, 912388580LL, 928469801LL, 944524620LL, 960552653LL, 976553524LL, 
    992526858LL, 1008472284LL, 1024389439LL, 1040277960LL, 1056137492LL, 1071967682LL, 
    1087768183LL, 1103538651LL, 1119278749LL, 1134988143LL, 1150666502LL, 1166313503LL, 
    1181928825LL, 1197512152LL, 1213063175LL, 1228581587LL, 1244067086LL, 1259519375LL, 
    1274938164LL, 1290323163LL, 1305674092LL, 1320990671LL, 1336272629LL, 1351519696LL, 
    1366731608LL, 1381908108LL, 1397048941LL, 1412153857LL, 1427222611LL, 1442254965LL, 
    1457250682LL, 1472209533LL, 1487131291LL, 1502015735LL, 1516862648LL, 1531671821LL, 
    1546443044LL, 1561176116LL, 1575870838LL, 1590527019LL, 1605144469LL, 1619723004LL, 
    1634262445LL, 1648762617LL, 1663223350LL, 1677644478LL, 1692025839LL, 1706367277LL, 
    1720668639LL, 1734929777LL, 1749150548LL, 1763330812LL, 1777470434LL, 1791569283LL, 
    1805627234LL, 1819644163LL, 1833619953LL, 1847554491LL, 1861447665LL, 1875299371LL, 
    1889109508LL, 1902877978LL, 1916604687LL, 1930289546LL, 1943932469LL, 1957533376LL, 
    1971092188LL, 1984608831LL, 1998083235LL, 2011515335LL, 2024905066LL, 2038252372LL, 
    2051557195LL, 2064819485LL, 2078039194LL, 2091216277LL, 2104350692LL, 2117442403LL, 
    2130491375LL, 2143497578LL, 2156460984LL, 2169381569LL, 2182259311LL, 2195094194LL, 
    2207886203LL, 2220635326LL, 2233341556LL, 2246004887LL, 2258625317LL, 2271202846LL, 
    2283737479LL, 2296229222LL, 2308678085LL, 2321084079LL, 2333447221LL, 2345767528LL, 
    2358045020LL, 2370279720LL, 2382471655LL, 2394620853LL, 2406727345LL, 2418791163LL, 
    2430812345LL, 2442790928LL, 2454726953LL, 2466620463LL, 2478471503LL, 2490280121LL, 
    2502046367LL, 2513770292LL, 2525451952LL, 2537091401LL, 2548688699LL, 2560243905LL, 
    2571757083LL, 2583228295LL, 2594657610LL, 2606045095LL, 2617390819LL, 2628694855LL, 
    2639957276LL, 2651178158LL, 2662357577LL, 2673495613LL, 2684592346LL, 2695647857LL, 
    2706662231LL, 2717635552LL, 2728567908LL, 2739459385LL, 2750310075LL, 2761120067LL, 
    2771889454LL, 2782618330LL, 2793306790LL, 2803954929LL, 2814562847LL, 2825130641LL, 
    2835658411LL, 2846146259LL, 2856594286LL, 2867002597LL, 2877371296LL, 2887700488LL, 
    2897990280LL, 2908240779LL, 2918452094LL, 2928624334LL, 2938757611LL, 2948852034LL, 
    2958907716LL, 2968924771LL, 2978903312LL, 2988843453LL, 2998745310LL, 3008608999LL, 
    3018434637LL, 3028222342LL, 3037972231LL, 3047684423LL, 3057359038LL, 3066996196LL, 
    3076596018LL, 3086158624LL, 3095684137LL, 3105172678LL, 3114624371LL, 3124039338LL, 
    3133417704LL, 3142759592LL, 3152065127LL, 3161334434LL, 3170567638LL, 3179764865LL, 
    3188926241LL, 3198051892LL, 3207141945LL, 3216196526LL, 3225215764LL, 3234199785LL, 
    3243148718LL, 3252062689LL, 3260941827LL, 3269786261LL, 3278596119LL, 3287371530LL, 
    3296112622LL, 3304819525LL, 3313492367LL, 3322131278LL, 3330736386LL, 3339307822LL, 
    3347845715LL, 3356350193LL, 3364821387LL, 3373259426LL
};

/**
 * @brief Lookup arctangent value for atan2 implementation with linear interpolation
 * @param ratio Fixed-point ratio value (y/x or x/y) in [0,1] range
 * @param P Precision (fractional bits) of the input
 * @return Fixed-point arctangent value with P fractional bits in [0, pi/4] range
 */
inline auto LookupAtan2Table(int64_t ratio, int P) noexcept -> int64_t {
    // Scale input to [0, 1] range in Q31.32 format
    constexpr int kTableP = 32;
    int64_t scaled_x;
    if (P > kTableP) {
        scaled_x = ratio >> (P - kTableP);
    } else if (P < kTableP) {
        scaled_x = ratio << (kTableP - P);
    } else {
        scaled_x = ratio;
    }

    // Ensure input is in valid range
    constexpr int64_t kOne = 1LL << kTableP;
    if (scaled_x >= kOne) {
        scaled_x = kOne - 1;
    }

    // Calculate table index and fractional part
    constexpr int kTableSize = 256;
    constexpr int64_t kIndexScale = (1LL << kTableP) / (kTableSize - 1);
    int index = static_cast<int>(scaled_x / kIndexScale);
    int64_t frac = scaled_x % kIndexScale;

    // Ensure index is within bounds
    if (index >= kTableSize - 1) {
        index = kTableSize - 2;
        frac = kIndexScale;
    }

    // Perform linear interpolation
    int64_t y0 = kAtan2LUT[index];
    int64_t y1 = kAtan2LUT[index + 1];
    int64_t result = y0 + ((y1 - y0) * frac) / kIndexScale;

    // Adjust precision if needed
    if (P > kTableP) {
        result = result << (P - kTableP);
    } else if (P < kTableP) {
        result = result >> (kTableP - P);
    }

    return result;
}

/**
 * @brief Fast lookup arctangent value for atan2 implementation with reduced precision
 * @param ratio Fixed-point ratio value (y/x or x/y) in [0,1] range
 * @param P Precision (fractional bits) of the input
 * @return Fixed-point arctangent value with P fractional bits in [0, pi/4] range
 */
inline auto LookupAtan2TableFast(int64_t ratio, int P) noexcept -> int64_t {
    // Scale input to [0, 1] range in Q31.32 format
    constexpr int kTableP = 32;
    int64_t scaled_x;
    if (P > kTableP) {
        scaled_x = ratio >> (P - kTableP);
    } else if (P < kTableP) {
        scaled_x = ratio << (kTableP - P);
    } else {
        scaled_x = ratio;
    }

    // Ensure input is in valid range
    constexpr int64_t kOne = 1LL << kTableP;
    if (scaled_x >= kOne) {
        scaled_x = kOne - 1;
    }

    // Calculate table index (simplified for speed)
    constexpr int kTableSize = 256;
    constexpr int kShift = kTableP - 8;  // Use 8 MSBs for indexing
    int index = static_cast<int>(scaled_x >> kShift);
    if (index >= kTableSize - 1) {
        index = kTableSize - 2;
    }

    // Get fractional part (8 bits)
    int64_t frac = (scaled_x >> (kShift - 8)) & 0xFF;

    // Perform linear interpolation
    int64_t y0 = kAtan2LUT[index];
    int64_t y1 = kAtan2LUT[index + 1];
    int64_t result = y0 + ((y1 - y0) * frac) / 256;

    // Adjust precision if needed
    if (P > kTableP) {
        result = result << (P - kTableP);
    } else if (P < kTableP) {
        result = result >> (kTableP - P);
    }

    return result;
}

} // namespace math::fp::detail
