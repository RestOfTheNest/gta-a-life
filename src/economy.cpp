#include "economy.h"
#include "economy_retail.h"
#include "economy_telemetry.h"
#include "municipal.h"
#include "crash_handler.h"

#include "CAutomobile.h"
#include "CClock.h"
#include "CColStore.h"
#include "CHud.h"
#include "CPed.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CStreaming.h"
#include "CTimer.h"
#include "CVehicle.h"
#include "CWanted.h"
#include "CWorld.h"
#include "common.h"
#include "enums/eModelID.h"
#include "enums/ePedType.h"
#include "enums/eWeaponType.h"
#include "extensions/ScriptCommands.h"
#include "plugin.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

#ifndef WEAPONTYPE_SPAS12_SHOTGUN
#define WEAPONTYPE_SPAS12_SHOTGUN WEAPONTYPE_SPAS12
#endif

#ifndef m_nMoney
#define m_nMoney GetMoney()
#endif

#ifndef m_nHandbrakeOn
#define m_nHandbrakeOn bIsHandbrakeOn
#endif

#ifndef m_nDrivingStyle
#define m_nDrivingStyle m_nCarDrivingStyle
#endif

#ifndef m_vecDestination
#define m_vecDestination m_vecDestinationCoors
#endif

static constexpr int STREAMING_PRIORITY_REQUEST = PRIORITY_REQUEST;
static constexpr int STREAMING_GAME_REQUIRED = GAME_REQUIRED;

using namespace plugin;
using namespace plugin::scripting;

// =============================================================================
//  Accurate Highway Waypoints & Logistics Fleet Specifications
// =============================================================================

const HighwayWaypoint s_highwayLoop[] = {
    {2312.2f, -2252.0f, 15.0f},  {2347.9f, -2222.3f, 15.0f},
    {2419.2f, -2172.3f, 15.0f},  {2697.5f, -2169.9f, 15.0f},
    {2766.5f, -2146.1f, 15.0f},  {2828.3f, -2089.1f, 15.0f},
    {2830.7f, -1996.3f, 15.0f},  {2833.1f, -1897.6f, 15.0f},
    {2840.2f, -1838.1f, 15.0f},  {2850.9f, -1733.5f, 15.0f},
    {2883.0f, -1599.1f, 15.0f},  {2909.2f, -1525.4f, 15.0f},
    {2916.3f, -1411.2f, 15.0f},  {2909.2f, -1337.5f, 15.0f},
    {2886.6f, -1279.2f, 15.0f},  {2883.0f, -1203.1f, 15.0f},
    {2884.4f, -1147.2f, 15.0f},  {2883.2f, -1077.1f, 15.0f},
    {2882.1f, -1021.2f, 15.0f},  {2886.8f, -932.0f, 15.0f},
    {2887.8f, -761.9f, 15.0f},   {2890.2f, -689.4f, 15.0f},
    {2881.8f, -556.2f, 15.0f},   {2840.2f, -500.3f, 15.0f},
    {2776.0f, -407.5f, 15.0f},   {2716.4f, -354.5f, 15.0f},
    {2698.8f, -289.8f, 15.0f},   {2743.3f, -167.9f, 15.0f},
    {2763.5f, -109.0f, 15.0f},   {2770.2f, 4.5f, 15.0f},
    {2777.0f, 51.6f, 15.0f},     {2772.8f, 90.3f, 15.0f},
    {2777.0f, 155.9f, 15.0f},    {2770.2f, 224.8f, 15.0f},
    {2721.5f, 293.8f, 15.0f},    {2679.4f, 316.5f, 15.0f},
    {2613.0f, 319.9f, 15.0f},    {2548.2f, 307.2f, 15.0f},
    {2493.6f, 310.6f, 15.0f},    {2441.4f, 320.7f, 15.0f},
    {2390.1f, 327.4f, 15.0f},    {2329.6f, 324.9f, 15.0f},
    {2255.6f, 324.9f, 15.0f},    {2169.8f, 323.2f, 15.0f},
    {2083.2f, 324.1f, 15.0f},    {2000.8f, 315.7f, 15.0f},
    {1908.3f, 302.2f, 15.0f},    {1833.5f, 282.0f, 15.0f},
    {1771.2f, 277.0f, 15.0f},    {1732.6f, 290.4f, 15.0f},
    {1710.7f, 310.6f, 15.0f},    {1700.6f, 345.9f, 15.0f},
    {1698.9f, 385.4f, 15.0f},    {1715.7f, 419.9f, 15.0f},
    {1736.8f, 482.2f, 15.0f},    {1757.8f, 543.5f, 15.0f},
    {1780.5f, 606.6f, 15.0f},    {1784.7f, 655.4f, 15.0f},
    {1791.4f, 697.4f, 15.0f},    {1801.5f, 765.5f, 15.0f},
    {1803.2f, 800.9f, 15.0f},    {1806.8f, 893.5f, 15.0f},
    {1809.7f, 950.5f, 15.0f},    {1808.6f, 1014.2f, 15.0f},
    {1804.4f, 1040.9f, 15.0f},   {1799.6f, 1109.9f, 15.0f},
    {1810.3f, 1195.5f, 15.0f},   {1803.2f, 1309.7f, 15.0f},
    {1810.3f, 1389.4f, 15.0f},   {1799.6f, 1501.1f, 15.0f},
    {1803.2f, 1577.3f, 15.0f},   {1820.0f, 1593.0f, 15.0f},
    {1835.5f, 1630.5f, 15.0f},   {1858.0f, 1649.5f, 15.0f},
    {1871.5f, 1678.0f, 15.0f},   {1872.2f, 1714.9f, 15.0f},
    {1846.1f, 1720.0f, 15.0f},   {1804.9f, 1714.1f, 15.0f},
    {1746.9f, 1710.7f, 15.0f},   {1686.3f, 1711.5f, 15.0f},
    {1635.9f, 1709.0f, 15.0f},   {1580.4f, 1707.3f, 15.0f},
    {1566.9f, 1723.3f, 15.0f},   {1567.7f, 1769.6f, 15.0f},
    {1569.4f, 1825.9f, 15.0f},   {1565.2f, 1869.6f, 15.0f},
    {1521.4f, 1872.2f, 15.0f},   {1490.4f, 1880.5f, 15.0f},
    {1496.4f, 1934.0f, 15.0f},   {1488.1f, 1953.0f, 15.0f},
    {1495.2f, 1969.7f, 15.0f},   {1572.5f, 1976.8f, 15.0f},
    {1567.7f, 2025.6f, 15.0f},   {1566.6f, 2051.8f, 15.0f},
    {1464.3f, 2042.2f, 15.0f},   {1404.8f, 2052.9f, 15.0f},
    {1320.4f, 2042.2f, 15.0f},   {1269.3f, 2054.1f, 15.0f},
    {1196.7f, 2045.8f, 15.0f},   {1139.6f, 2044.6f, 15.0f},
    {1067.1f, 2052.9f, 15.0f},   {1004.1f, 2047.0f, 15.0f},
    {1010.0f, 2003.0f, 15.0f},   {1011.2f, 1951.9f, 15.0f},
    {1007.6f, 1898.3f, 15.0f},   {1008.8f, 1841.3f, 15.0f},
    {1061.1f, 1812.7f, 15.0f},   {1112.3f, 1813.9f, 15.0f},
    {1140.8f, 1812.7f, 15.0f},   {1287.1f, 1808.0f, 15.0f},
    {1296.6f, 1825.8f, 15.0f},   {1265.7f, 1892.4f, 15.0f},
    {1228.8f, 1953.0f, 15.0f},   {1228.8f, 2089.8f, 15.0f},
    {1227.6f, 2154.0f, 15.0f},   {1251.4f, 2202.8f, 15.0f},
    {1290.7f, 2245.6f, 15.0f},   {1347.7f, 2325.3f, 15.0f},
    {1348.9f, 2370.5f, 15.0f},   {1308.5f, 2406.1f, 15.0f},
    {1258.6f, 2424.0f, 15.0f},   {1193.1f, 2439.4f, 15.0f},
    {1126.5f, 2473.9f, 15.0f},   {1076.6f, 2504.8f, 15.0f},
    {1013.6f, 2550.0f, 15.0f},   {941.0f, 2594.0f, 15.0f},
    {856.6f, 2633.3f, 15.0f},    {760.3f, 2655.9f, 15.0f},
    {622.3f, 2657.1f, 15.0f},    {454.6f, 2657.1f, 15.0f},
    {416.6f, 2696.3f, 15.0f},    {338.1f, 2714.1f, 15.0f},
    {228.7f, 2747.4f, 15.0f},    {165.7f, 2753.4f, 15.0f},
    {118.1f, 2720.1f, 15.0f},    {97.9f, 2699.9f, 15.0f},
    {51.5f, 2657.1f, 15.0f},     {-62.7f, 2640.4f, 15.0f},
    {-150.7f, 2634.5f, 15.0f},   {-245.8f, 2634.5f, 15.0f},
    {-336.2f, 2635.7f, 15.0f},   {-383.7f, 2690.4f, 15.0f},
    {-452.7f, 2721.3f, 15.0f},   {-521.7f, 2717.7f, 15.0f},
    {-574.0f, 2740.3f, 15.0f},   {-619.2f, 2759.3f, 15.0f},
    {-698.9f, 2739.1f, 15.0f},   {-840.4f, 2726.0f, 15.0f},
    {-991.4f, 2717.7f, 15.0f},   {-1138.9f, 2699.9f, 15.0f},
    {-1237.6f, 2679.7f, 15.0f},  {-1323.2f, 2647.5f, 15.0f},
    {-1347.6f, 2637.4f, 15.0f},  {-1379.2f, 2686.8f, 15.0f},
    {-1435.1f, 2721.3f, 15.0f},  {-1516.0f, 2729.6f, 15.0f},
    {-1618.3f, 2733.2f, 15.0f},  {-1694.4f, 2724.8f, 15.0f},
    {-1752.6f, 2727.2f, 15.0f},  {-1793.1f, 2696.3f, 15.0f},
    {-1809.3f, 2679.8f, 15.0f},  {-1822.8f, 2686.1f, 15.0f},
    {-1853.5f, 2677.3f, 15.0f},  {-1860.6f, 2658.8f, 15.0f},
    {-1878.3f, 2638.2f, 15.0f},  {-1901.0f, 2626.4f, 15.0f},
    {-1931.3f, 2612.6f, 15.0f},  {-1957.8f, 2607.5f, 15.0f},
    {-2009.0f, 2626.0f, 15.0f},  {-2053.0f, 2637.0f, 15.0f},
    {-2100.0f, 2659.0f, 15.0f},  {-2234.0f, 2673.0f, 15.0f},
    {-2287.0f, 2677.0f, 15.0f},  {-2390.0f, 2671.0f, 15.0f},
    {-2515.0f, 2669.0f, 15.0f},  {-2607.0f, 2668.0f, 15.0f},
    {-2680.0f, 2660.0f, 15.0f},  {-2741.0f, 2611.0f, 15.0f},
    {-2761.0f, 2565.0f, 15.0f},  {-2772.0f, 2513.0f, 15.0f},
    {-2778.0f, 2461.0f, 15.0f},  {-2780.0f, 2402.0f, 15.0f},
    {-2771.0f, 2354.0f, 15.0f},  {-2751.0f, 2297.0f, 15.0f},
    {-2724.0f, 2245.0f, 15.0f},  {-2700.0f, 2208.0f, 15.0f},
    {-2687.0f, 2163.0f, 15.0f},  {-2688.0f, 2113.0f, 15.0f},
    {-2683.0f, 2040.0f, 15.0f},  {-2690.0f, 2026.0f, 15.0f},
    {-2688.0f, 1985.0f, 15.0f},  {-2690.0f, 1936.0f, 15.0f},
    {-2693.0f, 1889.0f, 15.0f},  {-2689.0f, 1799.0f, 15.0f},
    {-2689.0f, 1700.0f, 15.0f},  {-2687.0f, 1599.0f, 15.0f},
    {-2686.0f, 1534.0f, 15.0f},  {-2684.0f, 1516.0f, 15.0f},
    {-2684.0f, 1444.0f, 15.0f},  {-2692.0f, 1362.0f, 15.0f},
    {-2687.0f, 1285.0f, 15.0f},  {-2671.0f, 1196.0f, 15.0f},
    {-2629.0f, 1148.0f, 15.0f},  {-2607.0f, 1128.0f, 15.0f},
    {-2537.0f, 1110.0f, 15.0f},  {-2474.0f, 1101.0f, 15.0f},
    {-2449.4f, 1093.3f, 15.0f},  {-2379.8f, 1078.4f, 15.0f},
    {-2316.2f, 1067.1f, 15.0f},  {-2286.5f, 1067.1f, 15.0f},
    {-2197.9f, 1065.9f, 15.0f},  {-2095.6f, 1065.3f, 15.0f},
    {-2042.7f, 1067.7f, 15.0f},  {-1993.0f, 1072.0f, 15.0f},
    {-1899.0f, 1059.0f, 15.0f},  {-1887.0f, 1043.0f, 15.0f},
    {-1894.0f, 990.0f, 15.0f},   {-1896.4f, 943.4f, 15.0f},
    {-1790.6f, 924.4f, 15.0f},   {-1691.9f, 922.0f, 15.0f},
    {-1607.4f, 920.8f, 15.0f},   {-1536.1f, 922.0f, 15.0f},
    {-1524.2f, 917.2f, 15.0f},   {-1527.8f, 873.2f, 15.0f},
    {-1536.1f, 844.7f, 15.0f},   {-1545.6f, 791.2f, 15.0f},
    {-1555.1f, 751.9f, 15.0f},   {-1553.9f, 700.8f, 15.0f},
    {-1561.1f, 599.7f, 15.0f},   {-1563.4f, 508.2f, 15.0f},
    {-1589.6f, 459.4f, 15.0f},   {-1626.5f, 424.9f, 15.0f},
    {-1677.6f, 370.2f, 15.0f},   {-1722.8f, 331.0f, 15.0f},
    {-1764.4f, 290.5f, 15.0f},   {-1801.3f, 251.3f, 15.0f},
    {-1807.8f, 214.5f, 15.0f},   {-1809.2f, 179.2f, 15.0f},
    {-1778.8f, 184.1f, 15.0f},   {-1762.6f, 177.0f, 15.0f},
    {-1765.4f, 141.7f, 15.0f},   {-1759.7f, 110.6f, 15.0f},
    {-1758.3f, 54.7f, 15.0f},    {-1761.9f, -16.7f, 15.0f},
    {-1761.1f, -100.2f, 15.0f},  {-1761.9f, -117.1f, 15.0f},
    {-1788.2f, -116.2f, 15.0f},  {-1797.2f, -136.9f, 15.0f},
    {-1795.8f, -194.9f, 15.0f},  {-1798.6f, -233.8f, 15.0f},
    {-1802.2f, -326.4f, 15.0f},  {-1813.2f, -390.9f, 15.0f},
    {-1826.3f, -468.2f, 15.0f},  {-1825.1f, -528.8f, 15.0f},
    {-1820.3f, -558.6f, 15.0f},  {-1814.4f, -607.3f, 15.0f},
    {-1820.3f, -666.8f, 15.0f},  {-1808.4f, -725.0f, 15.0f},
    {-1807.2f, -795.2f, 15.0f},  {-1807.2f, -849.9f, 15.0f},
    {-1809.6f, -929.6f, 15.0f},  {-1795.3f, -1040.2f, 15.0f},
    {-1766.8f, -1123.4f, 15.0f}, {-1737.1f, -1184.1f, 15.0f},
    {-1690.7f, -1266.1f, 15.0f}, {-1663.3f, -1316.1f, 15.0f},
    {-1620.5f, -1382.7f, 15.0f}, {-1567.0f, -1438.6f, 15.0f},
    {-1530.1f, -1442.1f, 15.0f}, {-1526.6f, -1391.0f, 15.0f},
    {-1537.3f, -1291.1f, 15.0f}, {-1617.0f, -1194.8f, 15.0f},
    {-1671.7f, -1154.4f, 15.0f}, {-1709.7f, -1106.8f, 15.0f},
    {-1737.1f, -1027.1f, 15.0f}, {-1746.6f, -958.1f, 15.0f},
    {-1752.5f, -904.6f, 15.0f},  {-1750.1f, -861.8f, 15.0f},
    {-1695.4f, -796.4f, 15.0f},  {-1658.6f, -791.6f, 15.0f},
    {-1628.8f, -826.1f, 15.0f},  {-1626.5f, -879.6f, 15.0f},
    {-1633.6f, -949.8f, 15.0f},  {-1638.4f, -1014.0f, 15.0f},
    {-1608.6f, -1123.4f, 15.0f}, {-1574.1f, -1166.2f, 15.0f},
    {-1534.9f, -1209.1f, 15.0f}, {-1480.2f, -1268.5f, 15.0f},
    {-1445.7f, -1328.0f, 15.0f}, {-1421.9f, -1408.8f, 15.0f},
    {-1404.1f, -1416.0f, 15.0f}, {-1358.9f, -1396.9f, 15.0f},
    {-1266.1f, -1368.4f, 15.0f}, {-1217.4f, -1352.9f, 15.0f},
    {-1171.0f, -1345.8f, 15.0f}, {-1129.4f, -1342.2f, 15.0f},
    {-1077.1f, -1351.8f, 15.0f}, {-1005.7f, -1382.7f, 15.0f},
    {-964.1f, -1398.1f, 15.0f},  {-924.8f, -1393.4f, 15.0f},
    {-911.8f, -1363.7f, 15.0f},  {-898.7f, -1319.7f, 15.0f},
    {-890.3f, -1186.5f, 15.0f},  {-877.3f, -1084.2f, 15.0f},
    {-817.8f, -1020.0f, 15.0f},  {-759.5f, -1003.3f, 15.0f},
    {-679.9f, -999.8f, 15.0f},   {-620.6f, -979.8f, 15.0f},
    {-583.1f, -953.6f, 15.0f},   {-538.6f, -925.3f, 15.0f},
    {-457.2f, -844.0f, 15.0f},   {-362.5f, -837.0f, 15.0f},
    {-314.4f, -865.2f, 15.0f},   {-246.5f, -892.1f, 15.0f},
    {-197.7f, -934.5f, 15.0f},   {-156.0f, -957.9f, 15.0f},
    {-105.8f, -998.9f, 15.0f},   {-88.1f, -1034.2f, 15.0f},
    {-90.0f, -1092.5f, 15.0f},   {-99.5f, -1132.9f, 15.0f},
    {-119.7f, -1180.5f, 15.0f},  {-140.0f, -1235.2f, 15.0f},
    {-149.5f, -1297.1f, 15.0f},  {-156.6f, -1356.5f, 15.0f},
    {-144.7f, -1421.9f, 15.0f},  {-100.7f, -1498.0f, 15.0f},
    {-17.5f, -1518.2f, 15.0f},   {65.8f, -1526.6f, 15.0f},
    {137.1f, -1555.1f, 15.0f},   {182.3f, -1608.6f, 15.0f},
    {256.0f, -1687.1f, 15.0f},   {322.0f, -1708.0f, 15.0f},
    {427.0f, -1711.0f, 15.0f},   {591.0f, -1726.0f, 15.0f},
    {663.0f, -1748.0f, 15.0f},   {861.0f, -1784.0f, 15.0f},
    {972.0f, -1780.0f, 15.0f},   {1060.0f, -1847.0f, 15.0f},
    {1060.0f, -1943.0f, 15.0f},  {1051.0f, -2009.0f, 15.0f},
    {1046.0f, -2057.0f, 15.0f},  {1045.0f, -2266.0f, 15.0f},
    {1138.0f, -2401.0f, 15.0f},  {1197.0f, -2429.0f, 15.0f},
    {1282.0f, -2460.0f, 15.0f},  {1343.0f, -2467.0f, 15.0f},
    {1341.6f, -2507.9f, 15.0f},  {1340.2f, -2594.1f, 15.0f},
    {1386.9f, -2666.2f, 15.0f},  {1504.3f, -2684.6f, 15.0f},
    {1593.4f, -2681.8f, 15.0f},  {1750.3f, -2687.5f, 15.0f},
    {1884.7f, -2677.6f, 15.0f},  {2023.3f, -2690.3f, 15.0f},
    {2125.1f, -2654.9f, 15.0f},  {2167.5f, -2616.7f, 15.0f},
    {2171.8f, -2537.6f, 15.0f},  {2170.4f, -2481.0f, 15.0f},
    {2173.2f, -2413.1f, 15.0f},  {2211.4f, -2355.1f, 15.0f},
    {2309.0f, -2256.1f, 15.0f}};

const size_t s_waypointCount = sizeof(s_highwayLoop) / sizeof(s_highwayLoop[0]);
const HighwayWaypoint *const k_highwayWaypoints = s_highwayLoop;

const int k_truckModels[3] = {MODEL_LINERUNNER, MODEL_ROADTRAIN, MODEL_TANKER};

const LogisticsCompany k_companies[3] = {
    {0, "Ocean Docks Logistics", "#38bdf8", "Ocean Docks"},
    {1, "Bone County Petroleum", "#f59e0b", "Bone County"},
    {2, "San Andreas Agro Food", "#22c55e", "Flint County Farms"}};

std::recursive_mutex g_customRoutesMutex;
CompanyCustomRoute g_customRoutes[3];
VirtualTruck s_fleet[k_totalTruckCount]{};

const LogisticsAsset k_assets[3] = {{0, "Ocean Docks Terminal", 250000},
                                    {1, "Bone County Truck Stop", 320000},
                                    {2, "Flint County Agro Silo", 280000}};

const char *const k_driverNames[30] = {
    "Hank",   "Bob",    "Frank",    "Trevor",    "CJ",        "Big Smoke",
    "Ryder",  "Claude", "Cesar",    "Sweet",     "Maccer",    "Kent",
    "Mike",   "Jethro", "Dwight",   "Zero",      "T-Bone",    "Toreno",
    "Woozie", "Wu Zi",  "Catalina", "The Truth", "Madd Dogg", "OG Loc",
    "Kendl",  "Brian",  "Beverly",  "Denise",    "Helena",    "Katie"};

const CVector k_gasStationCoords[3] = {{1944.5f, -1771.2f, 13.3f},
                                       {-91.3f, -1170.5f, 2.1f},
                                       {2117.8f, 2721.5f, 10.8f}};

const CVector k_shopCoords[2] = {{1365.2f, -1279.8f, 13.5f},
                                 {2165.1f, -1675.4f, 15.1f}};

static std::atomic<uint32_t> s_ticksSinceLastSfDelivery{0};

static inline void DeductCompanyBalance(uint8_t companyId, int64_t amount, const char* reason = nullptr) {
  if (companyId < 3) {
    s_companyBalances[companyId].fetch_sub(amount, std::memory_order_relaxed);
  }
}

// Density cap helper based on cargo type:
// 1: Timber (18.0f), 2: Fuel/Oil (26.0f), 3: Electronics (14.0f), 4: Food
// (20.0f), default: 20.0f
static inline constexpr float GetCargoDensityCap(int cargoType) noexcept {
  switch (cargoType) {
  case 1:
    return 18.0f; // Timber
  case 2:
    return 26.0f; // Fuel/Oil
  case 3:
    return 14.0f; // Electronics
  case 4:
    return 20.0f; // Food
  default:
    return 20.0f;
  }
}

// Loading and Compliance evaluation helper
static inline void ApplyCargoComplianceAndLoading(VirtualTruck &t) noexcept {
  const int64_t bal = s_companyBalances[t.companyId < 3 ? t.companyId : 0].load(
      std::memory_order_relaxed);
  if (bal < 5000) {
    t.complianceTier = ComplianceTier::GRAY_MARKET;
  } else if (bal < 15000) {
    t.complianceTier = ComplianceTier::WATCHLIST;
  } else {
    t.complianceTier = ComplianceTier::STANDARD;
  }

  // Overload chance: gray market or rookie skill (skillLevel == 1) -> 75%,
  // otherwise standard logic (15%)
  const bool isHighRisk =
      (t.complianceTier == ComplianceTier::GRAY_MARKET) || (t.skillLevel == 1);
  const int roll = rand() % 100;
  const bool shouldOverload = isHighRisk ? (roll < 75) : (roll < 15);

  if (shouldOverload) {
    t.cargoWeightTons +=
        8 + (rand() % 5); // Overload weight spike = +8..12 tons
  }

  const float cap = GetCargoDensityCap(t.cargoType);
  t.isOverloaded = (static_cast<float>(t.cargoWeightTons) > cap);
  g_physicalFeedback[t.id].isOverloaded.store(t.isOverloaded,
                                              std::memory_order_relaxed);
  g_physicalFeedback[t.id].cargoType.store(static_cast<uint8_t>(t.cargoType),
                                           std::memory_order_relaxed);
  g_physicalFeedback[t.id].cargoWeightTons.store(
      static_cast<uint32_t>(t.cargoWeightTons), std::memory_order_relaxed);
}

// Synchronous Roadside Pull-Over Helper (Resting & Breakdowns)
void PullTruckToRoadside(CAutomobile *veh, float offsetDist) {
  if (!veh)
    return;
  float heading = veh->GetHeading();
  float fwdX = std::sin(-heading);
  float fwdY = std::cos(-heading);
  float rightX = fwdY;
  float rightY = -fwdX;

  CVector pos = veh->GetPosition();
  pos.x += rightX * offsetDist;
  pos.y += rightY * offsetDist;
  float groundZ = CWorld::FindGroundZForCoord(pos.x, pos.y);
  if (groundZ > -100.0f)
    pos.z = groundZ;
  veh->Teleport(pos);
  veh->UpdateRwMatrix();

  if (veh->m_pTrailer) {
    CAutomobile *trailer = reinterpret_cast<CAutomobile *>(veh->m_pTrailer);
    CVector tPos = trailer->GetPosition();
    tPos.x += rightX * offsetDist;
    tPos.y += rightY * offsetDist;
    float tGroundZ = CWorld::FindGroundZForCoord(tPos.x, tPos.y);
    if (tGroundZ > -100.0f)
      tPos.z = tGroundZ;
    trailer->Teleport(tPos);
    trailer->UpdateRwMatrix();
  }
}

static void EvaluateDistrictLivingStandards(bool applyDeltas = true) {
  // --- Nominal Base Wages (per evaluation cycle) ---
  static const float k_districtBaseWages[4] = {
      1100.0f, // District 0: South Central — blue-collar service/cash work
      4200.0f, // District 1: Downtown — white-collar brokers/landlords
      2400.0f, // District 2: Industrial Port — logistics/dock workers
      1400.0f  // District 3: Country / Rural — farm/labor
  };
  static const char *const k_districtNames[4] = {
      "South Central", "Downtown", "Industrial Port", "Country / Rural"};
  // --- Family Size Weighting (dependency ratios) ---
  static const float k_familyMultiplier[4] = {
      1.15f, // Dist 0: Moderate dependency
      1.05f, // Dist 1: Mostly single young professionals
      1.15f, // Dist 2: Small working families
      1.15f  // Dist 3: Moderate rural households
  };

  const uint32_t totalFood =
      s_sfFoodStock.load(std::memory_order_relaxed) +
      (s_portFoodStock.load(std::memory_order_relaxed) / 2);
  const float scarcityMult =
      (totalFood < 50) ? 1.6f : ((totalFood < 100) ? 1.25f : 1.0f);
  const float salesGenTax = s_taxSalesGeneral.load(std::memory_order_relaxed);
  const float fuelPriceMul =
      s_fuelPriceMultiplier.load(std::memory_order_relaxed);
  const float crimeRate = s_crimeRate.load(std::memory_order_relaxed);
  const float haulerPortTax = s_taxHaulerPort.load(std::memory_order_relaxed);

  // --- Port logistics stock for District 2 bonus ---
  const uint32_t combinedPortStock =
      s_portFoodStock.load(std::memory_order_relaxed) +
      s_sfFoodStock.load(std::memory_order_relaxed);

  // --- Retail solvency count for District 1 dividend ---
  uint32_t solventStoreCount = 0;
  for (size_t i = 0; i < 20; ++i) {
    if (!g_retailStores[i].isBankrupt.load(std::memory_order_relaxed) &&
        g_retailStores[i].capitalBalance.load(std::memory_order_relaxed) >
            5000) {
      ++solventStoreCount;
    }
  }

  float sumNetRatio = 0.0f;
  for (uint32_t d = 0; d < 4; ++d) {
    auto &dist = g_districtLivingStandards[d];
    dist.id = d;
    strncpy_s(dist.name, k_districtNames[d], sizeof(dist.name) - 1);

    const float baseWage = k_districtBaseWages[d];

    // --- A. Dynamic Income Generation ---
    float dynamicBonus = 0.0f;
    if (d == 2) {
      // Port & Transport: Logistics surge up to +15% ($360) based on warehouse
      // stock & tariff pressure
      const float stockFrac = std::clamp(
          static_cast<float>(combinedPortStock) / 4000.0f, 0.0f, 1.0f);
      const float tariffPenalty =
          std::clamp(1.0f - (haulerPortTax / 0.40f), 0.0f, 1.0f);
      dynamicBonus = 360.0f * stockFrac * tariffPenalty;
    } else if (d == 1) {
      // Downtown: Capital dividend up to +20% ($840) based on retail solvency
      // If 70%+ stores (14/20) solvent -> max dividend; below 30% (6/20) ->
      // zero
      const float dividendFrac = std::clamp(
          (static_cast<float>(solventStoreCount) / 20.0f - 0.30f) / 0.40f, 0.0f,
          1.0f);
      dynamicBonus = 840.0f * dividendFrac;
    }
    // Districts 0 & 3: No bonuses (blue-collar / rural labor)

    dist.baseWage = baseWage;
    dist.dynamicBonus = dynamicBonus;
    dist.effectiveWage = baseWage + dynamicBonus;

    // --- B. Homeownership Housing Model (Rent vs Maintenance) ---
    dist.effectiveHousingCost =
        (static_cast<float>(dist.baseRent) * (1.0f - dist.homeownershipRate)) +
        (static_cast<float>(dist.baseMaintenance) * dist.homeownershipRate);
    dist.fixedLivingCost = dist.effectiveHousingCost;

    // --- C. Grocery & Survival Basket with Family Weighting ---
    const float avgPrice = GetDistrictAverageStorePrice(d, 0);
    const float rawBasket =
        (avgPrice * 6.5f * scarcityMult) * (1.0f + salesGenTax) +
        (fuelPriceMul * 75.0f);
    dist.basketCost = rawBasket * k_familyMultiplier[d];

    // --- D. Crime Surcharge ("The Ghetto Tax") ---
    const float crimePovMul = (d == 0 || d == 3) ? 1.3f : 0.6f;
    dist.crimeSurcharge = (crimeRate / 100.0f) * 130.0f * crimePovMul;

    // --- E. District Poll Tax ---
    const float localPollTax =
        s_districtPollTax[d].load(std::memory_order_relaxed);
    dist.pollTax = localPollTax * g_districtZones[d].wealthModifier;

    // --- F. Net Delta, Ratio & Status ---
    dist.totalDeduction = dist.fixedLivingCost + dist.basketCost + dist.pollTax +
                          dist.crimeSurcharge;

    const float netDelta = dist.effectiveWage - dist.totalDeduction;
    const float safeWage = (std::max)(dist.effectiveWage, 1.0f);
    dist.netRatio = netDelta / safeWage;

    // Multi-tiered status evaluation:
    // - DEFICIT: netDelta < 0.0f -> Red, austerity active, drives unrest/crime.
    // - VULNERABLE: netDelta >= 0.0f && netDelta < 250.0f -> Orange, austerity active (paycheck-to-paycheck).
    // - STRAINED: netDelta >= 250.0f && (netRatio < 0.15f || netDelta < 400.0f) -> Amber, stable poverty.
    // - PROSPEROUS: netRatio >= 0.15f && netDelta >= 400.0f -> Green, genuine capital accumulation.
    if (netDelta < 0.0f) {
      strncpy_s(dist.status, "DEFICIT", sizeof(dist.status) - 1);
      dist.austerity = true;
      if (applyDeltas) {
        dist.consecutiveDeficitCycles++;
        if (dist.householdSavingsPool < 0)
          dist.householdSavingsPool = 0;
      }
    } else if (netDelta < 250.0f) {
      strncpy_s(dist.status, "VULNERABLE", sizeof(dist.status) - 1);
      dist.austerity = true;
      if (applyDeltas) {
        dist.consecutiveDeficitCycles = 0;
      }
    } else if (dist.netRatio < 0.15f || netDelta < 400.0f) {
      strncpy_s(dist.status, "STRAINED", sizeof(dist.status) - 1);
      dist.austerity = false;
      if (applyDeltas) {
        dist.consecutiveDeficitCycles = 0;
      }
    } else {
      strncpy_s(dist.status, "PROSPEROUS", sizeof(dist.status) - 1);
      dist.austerity = false;
      if (applyDeltas) {
        dist.consecutiveDeficitCycles = 0;
        if (dist.householdSavingsPool > 1000000) {
          constexpr int32_t k_rentCaps[4] = {800, 3500, 2200, 950};
          dist.baseRent = (std::min)(
              k_rentCaps[d], static_cast<int32_t>(dist.baseRent * 1.005f));
        }
      }
    }

    if (applyDeltas) {
      // 1. Census-weighted demographic household scaling
      // D0 (South Central): 120, D1 (Downtown): 60, D2 (Port): 80, D3 (Rural): 50
      static constexpr float k_strataScale[4] = { 120.0f, 60.0f, 80.0f, 50.0f };
      dist.householdSavingsPool += static_cast<int64_t>(netDelta * k_strataScale[d]);
      if (dist.householdSavingsPool < 0) {
        dist.householdSavingsPool = 0;
      }

      // 2. Owner-occupied maintenance municipal property tax (15% of baseMaintenance)
      const float maintenanceTax = (static_cast<float>(dist.baseMaintenance) * dist.homeownershipRate) * 0.15f;
      const int64_t maintTaxInt = static_cast<int64_t>(maintenanceTax * k_strataScale[d]);
      if (maintTaxInt > 0) {
        s_cityTreasury.fetch_add(maintTaxInt, std::memory_order_relaxed);
      }

      // 3. Gross tenant rental volume across the district
      const float rawRentFlow = static_cast<float>(dist.baseRent) * (1.0f - dist.homeownershipRate);
      const int64_t totalRentPool = static_cast<int64_t>(rawRentFlow * k_strataScale[d]);

      if (totalRentPool > 0) {
        // A. 12% Municipal Rental Income Tax -> City Treasury
        const int64_t rentTax = static_cast<int64_t>(totalRentPool * 0.12f);
        s_cityTreasury.fetch_add(rentTax, std::memory_order_relaxed);

        const int64_t netRentPool = totalRentPool - rentTax;

        // B. Mortgage Debt vs Private Outright Landlords:
        // Leveraged debt ratio by district: Downtown (50%), Port (35%), South Central (15%), Rural (15%)
        static constexpr float k_mortgageLeverage[4] = { 0.15f, 0.50f, 0.35f, 0.15f };
        const int64_t bankDebtService = static_cast<int64_t>(netRentPool * k_mortgageLeverage[d]);
        if (bankDebtService > 0) {
          g_fleecaBank.totalReserves.fetch_add(bankDebtService, std::memory_order_relaxed);
        }

        // C. Private landlord disposable consumer income (post-debt, post-tax)
        int64_t landlordDisposable = netRentPool - bankDebtService;

        // D. Capital flight: 35% of South Central landlord profits bleed into Downtown retail
        int64_t localExpenditure = landlordDisposable;
        int64_t downtownFlightCapital = 0;
        if (d == 0) {
          downtownFlightCapital = static_cast<int64_t>(landlordDisposable * 0.35f);
          localExpenditure -= downtownFlightCapital;
        }

        // Helper: Inject consumer spend into district retail nodes weighted by sales velocity
        auto injectConsumerSpend = [](uint32_t targetDist, int64_t spendCapital) {
          if (spendCapital <= 0) return;

          float totalVelocity = 0.0f;
          size_t eligibleCount = 0;
          for (size_t s = 0; s < 20; ++s) {
            if (g_retailStores[s].districtId == targetDist &&
                !g_retailStores[s].isBankrupt.load(std::memory_order_relaxed) &&
                g_retailStores[s].localStock.load(std::memory_order_relaxed) > 0) {
              const float vel = (std::max)(0.5f, g_retailStores[s].salesVelocityPerMin.load(std::memory_order_relaxed));
              totalVelocity += vel;
              eligibleCount++;
            }
          }

          if (eligibleCount == 0) {
            // Fallback: If all local retail insolvent, unspent capital returns to Fleeca reserves
            g_fleecaBank.totalReserves.fetch_add(spendCapital, std::memory_order_relaxed);
            return;
          }

          for (size_t s = 0; s < 20; ++s) {
            if (g_retailStores[s].districtId == targetDist &&
                !g_retailStores[s].isBankrupt.load(std::memory_order_relaxed) &&
                g_retailStores[s].localStock.load(std::memory_order_relaxed) > 0) {
              const float vel = (std::max)(0.5f, g_retailStores[s].salesVelocityPerMin.load(std::memory_order_relaxed));
              const int64_t storeCut = static_cast<int64_t>(spendCapital * (vel / totalVelocity));
              if (storeCut > 0) {
                g_retailStores[s].capitalBalance.fetch_add(storeCut, std::memory_order_relaxed);
              }
            }
          }
        };

        // Distribute local consumer spend to local retailers
        injectConsumerSpend(d, localExpenditure);

        // Distribute capital flight into Downtown commercial retailers
        if (downtownFlightCapital > 0) {
          injectConsumerSpend(1, downtownFlightCapital);
        }
      }
    }

    sumNetRatio += dist.netRatio;
  }

  if (!applyDeltas)
    return;

  const float avgR = sumNetRatio / 4.0f;

  // Dynamic crisis acceleration pacing (recalibrated for tight savings
  // margins):
  // - R < -0.30: Severe starvation/crisis: +4.8% unrest per 60s cycle (riot in
  // ~3-5 min)
  // - -0.30 <= R < 0.0: Moderate deficit: +1.8% per cycle
  // - 0.0 <= R < 0.12: Mild strain (paycheck-to-paycheck): +0.10% per cycle
  // - R >= 0.12: Prosperity: natural pacification -0.50% per cycle (floor 5%)
  float targetUnrestDelta = 0.0f;
  float targetCrimeDelta = 0.0f;
  float maxPositiveUnrestSlew = 0.50f;
  float maxNegativeUnrestSlew = -0.50f;

  const bool impoverishedInDeficit =
      (g_districtLivingStandards[0].netRatio < 0.0f ||
       g_districtLivingStandards[3].netRatio < 0.0f);

  if (avgR < -0.30f) {
    // Severe starvation / crisis: rapid 3-5 min escalation
    targetUnrestDelta = 4.8f;
    maxPositiveUnrestSlew = 5.5f;
    targetCrimeDelta = impoverishedInDeficit ? 2.5f : 1.5f;
  } else if (avgR < 0.0f) {
    // Moderate deficit
    targetUnrestDelta = 1.8f;
    maxPositiveUnrestSlew = 2.0f;
    targetCrimeDelta = impoverishedInDeficit ? 1.2f : 0.6f;
  } else if (avgR < 0.12f) {
    // Mild economic strain — paycheck to paycheck
    targetUnrestDelta = 0.10f;
    maxPositiveUnrestSlew = 0.20f;
    targetCrimeDelta = impoverishedInDeficit ? 0.15f : 0.05f;
  } else {
    // Prosperity: natural pacification decay down to min 5.0%
    targetUnrestDelta = -0.50f;
    maxNegativeUnrestSlew = -1.00f;
    targetCrimeDelta = -0.35f;
  }

  const float actualUnrestDelta = std::clamp(
      targetUnrestDelta, maxNegativeUnrestSlew, maxPositiveUnrestSlew);
  const float actualCrimeDelta = std::clamp(targetCrimeDelta, -1.2f, 3.0f);

  const float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);
  const float nextUnrest =
      (actualUnrestDelta < 0.0f)
          ? (std::max)(5.0f, curUnrest + actualUnrestDelta)
          : (std::min)(100.0f, curUnrest + actualUnrestDelta);
  s_socialUnrest.store(nextUnrest, std::memory_order_relaxed);
  s_publicUnrest.store(nextUnrest, std::memory_order_relaxed);

  const float curCrime = s_crimeRate.load(std::memory_order_relaxed);
  const float nextCrime = (actualCrimeDelta < 0.0f)
                              ? (std::max)(10.0f, curCrime + actualCrimeDelta)
                              : (std::min)(100.0f, curCrime + actualCrimeDelta);
  s_crimeRate.store(nextCrime, std::memory_order_relaxed);

  Logger::Log(
      "[LivingStandards] Avg Net Ratio: %.2f (D0: %.2f, D1: %.2f, D2: %.2f, "
      "D3: %.2f) | Unrest Delta: %+.2f -> %.1f%%, Crime Delta: %+.2f -> %.1f%%",
      avgR, g_districtLivingStandards[0].netRatio,
      g_districtLivingStandards[1].netRatio,
      g_districtLivingStandards[2].netRatio,
      g_districtLivingStandards[3].netRatio, actualUnrestDelta, nextUnrest,
      actualCrimeDelta, nextCrime);

  if (avgR < -0.30f) {
    AddMunicipalLog("CRITICAL FAMINE: Severe systemic living deficit (Net "
                    "Ratio %.2f). Unrest exploding (+%.2f%%).",
                    avgR, actualUnrestDelta);
  } else if (avgR < 0.0f) {
    AddMunicipalLog("CIVIC CRISIS: Living standard deficit (Net Ratio %.2f). "
                    "Unrest rising (+%.2f%%).",
                    avgR, actualUnrestDelta);
  } else if (avgR >= 0.12f && actualUnrestDelta < 0.0f) {
    AddMunicipalLog("PROSPERITY: High citizen real wage power (Net Ratio "
                    "%.2f). Unrest dropping (%.1f%%).",
                    avgR, nextUnrest);
  }
}

// =============================================================================
//  Background AI Director & Logistics Simulation Loop
// =============================================================================

void AIDirectorLoop() {
  uint32_t directiveIdCounter = 0;

  InitDistrictHousingDefaults();

  EconomyRetail::Init();

  EvaluateDistrictLivingStandards(false);

  InitCustomRoutes();

  // Initialize the two-tier logistics fleet:
  // Tier 1: 24 Interstate Highway Haulers (IDs 0..23) on master highway loop
  // Tier 2: 6 Dedicated Local Feeder Shuttles (IDs 24..29) for
  // production-to-port supply
  VirtualTruck(&fleet)[k_totalTruckCount] = s_fleet;
  for (size_t i = 0; i < k_truckCount; ++i) {
    fleet[i].id = static_cast<uint32_t>(i);
    fleet[i].speed = 22.0f; // Standard 22 m/s cruising speed
    fleet[i].isMaterialized = false;
    fleet[i].spawnRequested = false;
    fleet[i].physicalHandle = 0;
    fleet[i].state = TruckState::EN_ROUTE;
    fleet[i].stateTimer = 0;
    fleet[i].fatigue = static_cast<float>(10 + (i * 17) % 65);
    fleet[i].fuel = static_cast<float>(60 + (i * 13) % 40);
    fleet[i].companyId = static_cast<uint8_t>(i % 3);
    fleet[i].driverName = k_driverNames[i % 30];
    fleet[i].driverWallet = 0;
    fleet[i].experience = 0;
    fleet[i].skillLevel = 1;
    fleet[i].travelForward = true;

    // Distribute trucks evenly along the authentic master highway loop
    const size_t wpIdx = (i * s_waypointCount) / k_truckCount;
    const size_t nextWpIdx = (wpIdx + 1) % s_waypointCount;
    fleet[i].x = s_highwayLoop[wpIdx].x;
    fleet[i].y = s_highwayLoop[wpIdx].y;
    fleet[i].z = s_highwayLoop[wpIdx].z;
    fleet[i].currentTargetNode = nextWpIdx;
    fleet[i].travelForward = true;
    g_physicalFeedback[i].targetWaypointIndex.store(
        static_cast<uint32_t>(nextWpIdx), std::memory_order_release);
    g_physicalFeedback[i].travelForward.store(true, std::memory_order_release);

    const float dx = s_highwayLoop[nextWpIdx].x - fleet[i].x;
    const float dy = s_highwayLoop[nextWpIdx].y - fleet[i].y;
    float hRad = std::atan2(-dx, dy);
    float hDeg = hRad * (180.0f / 3.14159265358979323846f);
    while (hDeg < 0.0f)
      hDeg += 360.0f;
    while (hDeg >= 360.0f)
      hDeg -= 360.0f;
    fleet[i].heading = hDeg;

    if (fleet[i].companyId == 2) {
      fleet[i].cargoType = 4; // Food & Produce strictly for Agro Logistics
      fleet[i].cargoWeightTons = 16 + (static_cast<int>(i) % 7); // 16-22 tons
    } else if (fleet[i].companyId == 1) {
      fleet[i].cargoType = 2; // Fuel strictly for Petroleum
      fleet[i].cargoWeightTons = 20 + (static_cast<int>(i) % 7); // 20-26 tons
    } else {
      uint32_t tStock = s_sfTimberStock.load(std::memory_order_relaxed);
      uint32_t eStock = s_sfElectronicsStock.load(std::memory_order_relaxed);
      fleet[i].cargoType = (tStock <= eStock) ? 1 : 3;
      fleet[i].cargoWeightTons = 14 + (static_cast<int>(i) % 5);
    }
    ApplyCargoComplianceAndLoading(fleet[i]);
    fleet[i].deliveriesDone = 0;
  }

  // Initialize Dedicated Local Production Feeders for ALL 3 Companies
  // (IDs 24..29): IDs 24..25: Company 0 (Ocean Docks Logistics) IDs 26..27:
  // Company 1 (Bone County Petroleum) IDs 28..29: Company 2 (San Andreas Agro
  // Food)
  for (size_t i = k_truckCount; i < k_totalTruckCount; ++i) {
    fleet[i].id = static_cast<uint32_t>(i);
    fleet[i].speed = 20.0f;
    fleet[i].isMaterialized = false;
    fleet[i].spawnRequested = false;
    fleet[i].physicalHandle = 0;
    fleet[i].fatigue = static_cast<float>(5 + (i * 11) % 40);
    fleet[i].fuel = static_cast<float>(80 + (i * 7) % 20);
    fleet[i].driverName = k_driverNames[i % 30];
    fleet[i].driverWallet = 0;
    fleet[i].experience = 0;
    fleet[i].skillLevel = 1;
    fleet[i].deliveriesDone = 0;

    const uint8_t compId = static_cast<uint8_t>((i - k_truckCount) / 2);
    fleet[i].companyId = compId;

    // Automatically assign commodities based on company:
    // Company 0 (Ocean Docks Logistics): Intermodal Freight / Containers (+20
    // tons) Company 1 (Bone County Petroleum): Petrochemical / High-Octane Fuel
    // (+25 tons) Company 2 (San Andreas Agro Food): Fresh Agricultural Produce
    // / Grain (+25 tons)
    if (compId == 2) {
      fleet[i].cargoType = 4; // Fresh Agricultural Produce / Grain
      fleet[i].cargoWeightTons = 25;
    } else if (compId == 1) {
      fleet[i].cargoType = 2; // Petrochemical / High-Octane Fuel
      fleet[i].cargoWeightTons = 25;
    } else {
      fleet[i].cargoType = 3; // Intermodal Freight / Containers
      fleet[i].cargoWeightTons = 20;
    }

    // Spawn at Node 0 coordinates of their assigned route
    const auto &rNodes = g_customRoutes[compId].nodes;
    if (!rNodes.empty()) {
      fleet[i].x = rNodes[0].x;
      fleet[i].y = rNodes[0].y;
      fleet[i].z = rNodes[0].z;
    } else {
      fleet[i].x = s_highwayLoop[0].x;
      fleet[i].y = s_highwayLoop[0].y;
      fleet[i].z = s_highwayLoop[0].z;
    }

    const size_t subIdx = (i - k_truckCount) % 2;
    if (subIdx == 0) {
      // First company shuttle: en-route to unloading terminal
      fleet[i].state = TruckState::EN_ROUTE;
      fleet[i].stateTimer = 0;
      fleet[i].travelForward = true;
      fleet[i].currentTargetNode = (rNodes.size() > 1) ? 1 : 0;
    } else {
      // Second company shuttle: starts at Node 0 loading cargo, staged for
      // departure (6.0s)
      fleet[i].state = TruckState::LOADING;
      fleet[i].stateTimer = 120; // 6.0 seconds (120 ticks)
      fleet[i].travelForward = true;
      fleet[i].currentTargetNode = 0;
      fleet[i].cargoWeightTons = 0;
      fleet[i].cargoType = 0;
    }

    g_physicalFeedback[i].companyId.store(compId, std::memory_order_release);
    g_physicalFeedback[i].targetWaypointIndex.store(
        static_cast<uint32_t>(fleet[i].currentTargetNode),
        std::memory_order_release);
    g_physicalFeedback[i].travelForward.store(fleet[i].travelForward,
                                              std::memory_order_release);
    ApplyCargoComplianceAndLoading(fleet[i]);
  }

  auto lastTime = std::chrono::steady_clock::now();

  EconomyTelemetry::SerializeTelemetryJson();
  EconomyTelemetry::StartTelemetryWorker();

  while (g_running.load(std::memory_order_relaxed)) {
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    if (dt > 0.2f)
      dt = 0.05f;

    static uint32_t s_simTickCount = 0;
    s_simTickCount++;

    // Track local player presence and evaluate spatial district (thread-safe atomic mirror)
    CVector playerPos(
        g_playerPosX.load(std::memory_order_relaxed),
        g_playerPosY.load(std::memory_order_relaxed),
        g_playerPosZ.load(std::memory_order_relaxed));
    if (s_simTickCount % 50 == 0) {
      ENGINE_TRACE("ECONOMY", "AIDirectorLoop: simTick=%u playerPos=(%.1f, %.1f)", s_simTickCount, playerPos.x, playerPos.y);
    }
    s_playerCurrentDistrict.store(
        static_cast<uint32_t>(GetDistrictByCoords(playerPos.x, playerPos.y)),
        std::memory_order_relaxed);

    auto handleSfUnload = [&](VirtualTruck &t) {
      if (t.cargoType == 0 || t.cargoWeightTons == 0) return;

      std::atomic<uint32_t> *pCentralStock = nullptr;
      float basePayoutPerTon = 45.0f;
      constexpr uint32_t k_maxWarehouseCapacity = 800;

      if (t.cargoType == 1) { // Timber
        pCentralStock = &s_sfTimberStock;
        basePayoutPerTon = 40.0f;
      } else if (t.cargoType == 2) { // Fuel
        pCentralStock = &s_sfFuelStock;
        basePayoutPerTon = 65.0f;
      } else if (t.cargoType == 3) { // Electronics
        pCentralStock = &s_sfElectronicsStock;
        basePayoutPerTon = 90.0f;
      } else if (t.cargoType == 4) { // Food
        pCentralStock = &s_sfFoodStock;
        basePayoutPerTon = 50.0f;
      }

      if (!pCentralStock) return;

      // 1. Quadratic price elasticity relative to warehouse capacity
      const uint32_t curStock = pCentralStock->load(std::memory_order_relaxed);
      const float fillRatio = std::clamp(static_cast<float>(curStock) / static_cast<float>(k_maxWarehouseCapacity), 0.0f, 1.0f);
      const float priceElasticity = std::max(0.15f, 1.0f - (fillRatio * fillRatio));

      // 2. Demurrage queue penalty at the terminal
      uint32_t waitingTrucksAtSf = 0;
      for (size_t i = 0; i < k_totalTruckCount; ++i) {
        if (s_fleet[i].state == TruckState::UNLOADING && s_fleet[i].id != t.id) {
          waitingTrucksAtSf++;
        }
      }
      const float demurragePenalty = (waitingTrucksAtSf > 1) ? static_cast<float>(waitingTrucksAtSf - 1) * 350.0f : 0.0f;

      // 3. Normalized gross and net delivery payout with inland logistics bonus (+20% margin) to Easter Basin hub
      const float grossVal = static_cast<float>(t.cargoWeightTons) * basePayoutPerTon * priceElasticity * 1.5f * 1.20f;
      const int64_t netPayout = std::max<int64_t>(200, static_cast<int64_t>(grossVal - demurragePenalty));

      // 4. Warehouse accumulation with hard ceiling clamp: do not accept cargo beyond maxCapacity
      if (!s_portStrikeActive.load(std::memory_order_relaxed)) {
        const uint32_t current = pCentralStock->load(std::memory_order_relaxed);
        const uint32_t maxCap = k_maxWarehouseCapacity;
        const uint32_t cargoTons = t.cargoWeightTons * 2;
        if (current < maxCap) {
          const uint32_t availableSpace = maxCap - current;
          const uint32_t actualDelivered = (cargoTons > availableSpace) ? availableSpace : cargoTons;
          pCentralStock->fetch_add(actualDelivered, std::memory_order_relaxed);
        }
      }

      s_companyBalances[t.companyId].fetch_add(netPayout, std::memory_order_relaxed);

      // Fuel and maintenance deductions
      const float fuelCost = static_cast<float>(t.cargoWeightTons) * 12.0f * s_fuelPriceMultiplier.load(std::memory_order_relaxed);
      const int64_t opEx = static_cast<int64_t>(fuelCost) + 150;
      s_companyBalances[t.companyId].fetch_sub(opEx, std::memory_order_relaxed);

      const uint32_t driverCut = static_cast<uint32_t>((netPayout * 10) / 100);
      t.driverWallet += driverCut;
      t.deliveriesDone++;
      t.experience += 100;
      if (t.experience >= 800) t.skillLevel = 3;
      else if (t.experience >= 300) t.skillLevel = 2;

      Logger::Log("[Logistics] SF Unload #%u (%s): Net Payout: $%lld (Demurrage: -$%.0f, Stock: %u/%u)",
          t.id, k_companies[t.companyId].name, static_cast<long long>(netPayout), demurragePenalty,
          pCentralStock->load(std::memory_order_relaxed), k_maxWarehouseCapacity);

      // Return freight assignment
      if (t.companyId == 2) {
        t.cargoType = 4;
        t.cargoWeightTons = 8;
      } else if (t.companyId == 1) {
        t.cargoType = 2;
        t.cargoWeightTons = 6;
      } else {
        t.cargoType = 3;
        t.cargoWeightTons = 10;
      }
      t.deadlineTicks = 33600;
      t.deadlinePenalized = false;
      ApplyCargoComplianceAndLoading(t);
      s_ticksSinceLastSfDelivery.store(0, std::memory_order_relaxed);
    };

    auto handleOceanDocksArrival = [&](VirtualTruck &t) {
      if (t.cargoType != 0 && t.cargoWeightTons > 0) {
        uint32_t fuelStock = s_sfFuelStock.load(std::memory_order_relaxed);
        float scarcityBonus =
            (fuelStock < 25) ? 1.8f : (fuelStock > 150 ? 0.7f : 1.0f);
        uint32_t pricePerTon = 90;
        if (t.cargoType == 2) {
          pricePerTon = static_cast<uint32_t>(90.0f * scarcityBonus);
        }
        const float crisisCargoMul =
            s_activeCrisis.cargoPriceMul.load(std::memory_order_relaxed);
        if (crisisCargoMul != 1.0f) {
          pricePerTon = static_cast<uint32_t>(static_cast<float>(pricePerTon) *
                                              crisisCargoMul);
        }
        float qualityTier = 1.0f;
        if (!t.deadlinePenalized && t.deadlineTicks > 10000)
          qualityTier += 0.25f;
        else if (t.deadlinePenalized || t.deadlineTicks == 0)
          qualityTier -= 0.35f;
        qualityTier = std::clamp(qualityTier, 0.5f, 1.5f);

        uint32_t payout = static_cast<uint32_t>(t.cargoWeightTons *
                                                pricePerTon * qualityTier);
        bool hasAssetBonus = false;
        for (size_t a = 0; a < 3; ++a) {
          if (s_assetOwners[a].load(std::memory_order_relaxed) ==
              static_cast<int8_t>(t.companyId)) {
            hasAssetBonus = true;
            break;
          }
        }
        if (hasAssetBonus)
          payout = (payout * 125) / 100;

        // Progressive municipal harbor tariff: 30% routed to city treasury
        const float activeHaulerTaxRate =
            s_taxHaulerPort.load(std::memory_order_relaxed);
        const uint32_t municipalTax = (payout * 30) / 100;
        s_cityTreasury.fetch_add(static_cast<int64_t>(municipalTax),
                                 std::memory_order_relaxed);
        payout = (payout > municipalTax) ? (payout - municipalTax) : 0;

        s_companyBalances[t.companyId].fetch_add(payout,
                                                 std::memory_order_relaxed);

        // Операционные вычеты компании за рейс (расход топлива + амортизация/ТО
        // $120):
        const float fuelCost =
            static_cast<float>(t.cargoWeightTons) * 15.0f *
            s_fuelPriceMultiplier.load(std::memory_order_relaxed);
        const int64_t operatingCosts = static_cast<int64_t>(fuelCost) + 120;
        s_companyBalances[t.companyId].fetch_sub(operatingCosts,
                                                 std::memory_order_relaxed);

        uint32_t driverCut = (payout * 12) / 100;
        // Side effect: If activeHaulerTaxRate > 0.22f, hauler driver tips are
        // penalized and company strike chance increases if liquidity is tight.
        if (activeHaulerTaxRate > 0.22f) {
          const float penalty =
              (std::min)(0.5f, (activeHaulerTaxRate - 0.22f) * 2.5f);
          driverCut = static_cast<uint32_t>(static_cast<float>(driverCut) *
                                            (1.0f - penalty));
          if (s_companyBalances[t.companyId].load(std::memory_order_relaxed) <
              50000) {
            if ((rand() % 100) < 25) {
              s_companyStrikeTicks[t.companyId].store(
                  1200, std::memory_order_relaxed); // 60s strike
              Logger::Log("[MunicipalTax] Excessive port hauler tax (%.1f%%) "
                          "sparked strike in Company #%u!",
                          activeHaulerTaxRate * 100.0f, t.companyId);
            }
          }
        }

        t.driverWallet += driverCut;
        t.deliveriesDone++;
        t.experience += 100;
        if (t.experience >= 800) {
          t.skillLevel = 3;
        } else if (t.experience >= 300) {
          t.skillLevel = 2;
        }

        Logger::Log("[Logistics] Truck #%u (%s - %s) delivered return freight "
                    "to Ocean Docks: %d tons (Type %d), Net Payout: $%u (Tariff: "
                    "$%u, Costs: $%lld, Driver +$%u)",
                    t.id, t.driverName, k_companies[t.companyId].name,
                    t.cargoWeightTons, t.cargoType, payout, municipalTax,
                    static_cast<long long>(operatingCosts), driverCut);
      }

      t.state = TruckState::LOADING;
      t.stateTimer = 1600;
      t.speed = 0.0f;

      // Dispatch check: trucks should not target hubs that are >= 95% full
      std::atomic<uint32_t>* pTargetHubStock = nullptr;
      if (t.companyId == 2) pTargetHubStock = &s_sfFoodStock;
      else if (t.companyId == 1) pTargetHubStock = &s_sfFuelStock;
      else pTargetHubStock = &s_sfElectronicsStock;

      constexpr uint32_t k_sfMaxCap = 500;
      if (pTargetHubStock && pTargetHubStock->load(std::memory_order_relaxed) >= (k_sfMaxCap * 95) / 100) {
        // SF hub is >= 95% full: hold at Ocean Docks
        return;
      }

      if (t.companyId == 2) {
        // Agro Food: pull batch from Ocean Docks central port stock (deposited
        // by feeders)!
        uint32_t portStock = s_portFoodStock.load(std::memory_order_relaxed);
        uint32_t desired = 16 + (static_cast<int>(t.id) % 7); // 16-22 tons
        uint32_t availableTons = portStock / 4;
        uint32_t loadTons = (std::min)(desired, availableTons);
        if (loadTons > 0) {
          s_portFoodStock.fetch_sub(loadTons * 4, std::memory_order_relaxed);
          t.cargoType = 4;
          t.cargoWeightTons = loadTons;
          Logger::Log("[Logistics] Master Highway Truck #%u loaded %u tons of "
                      "Food from Ocean Docks reserves (Remaining: %u)",
                      t.id, loadTons,
                      s_portFoodStock.load(std::memory_order_relaxed));
        } else {
          t.cargoType = 4;
          t.cargoWeightTons =
              4; // Baseline batch while waiting for feeder replenishment
        }
      } else if (t.companyId == 1) {
        // Petroleum Fuel: pull batch from Ocean Docks central port stock
        // (deposited by feeders)!
        uint32_t portStock = s_portFuelStock.load(std::memory_order_relaxed);
        uint32_t desired = 20 + (static_cast<int>(t.id) % 7); // 20-26 tons
        uint32_t availableTons = portStock / 4;
        uint32_t loadTons = (std::min)(desired, availableTons);
        if (loadTons > 0) {
          s_portFuelStock.fetch_sub(loadTons * 4, std::memory_order_relaxed);
          t.cargoType = 2;
          t.cargoWeightTons = loadTons;
          Logger::Log("[Logistics] Master Highway Truck #%u loaded %u tons of "
                      "Fuel from Ocean Docks reserves (Remaining: %u)",
                      t.id, loadTons,
                      s_portFuelStock.load(std::memory_order_relaxed));
        } else {
          t.cargoType = 2;
          t.cargoWeightTons =
              5; // Baseline batch while waiting for feeder replenishment
        }
      } else {
        // Company 0: allocate lowest stock between Timber and Electronics
        uint32_t tStock = s_sfTimberStock.load(std::memory_order_relaxed);
        uint32_t eStock = s_sfElectronicsStock.load(std::memory_order_relaxed);
        t.cargoType = (tStock <= eStock) ? 1 : 3;
        t.cargoWeightTons = 14 + (static_cast<int>(t.id) % 5);
      }
      t.deadlineTicks = 33600;
      t.deadlinePenalized = false;
      ApplyCargoComplianceAndLoading(t);
    };

    auto handleWeighStationInspection = [&](VirtualTruck &t) {
      const float cap = GetCargoDensityCap(t.cargoType);
      const bool isOverweight = (static_cast<float>(t.cargoWeightTons) > cap);
      t.isOverloaded = isOverweight;
      g_physicalFeedback[t.id].isOverloaded.store(isOverweight,
                                                  std::memory_order_relaxed);

      const float storeMult =
          s_storePriceMultiplier.load(std::memory_order_relaxed);
      const uint32_t storeMultInt =
          std::max(1u, static_cast<uint32_t>(std::round(storeMult)));

      const uint32_t roadFee =
          isOverweight
              ? static_cast<uint32_t>(t.cargoWeightTons * 35 * storeMultInt +
                                      1200)
              : static_cast<uint32_t>(t.cargoWeightTons * 12 * storeMultInt);

      // Underflow-safe floor-0 balance deduction
      int64_t curBal =
          s_companyBalances[t.companyId].load(std::memory_order_relaxed);
      while (curBal > 0 &&
             !s_companyBalances[t.companyId].compare_exchange_weak(
                 curBal,
                 (curBal >= static_cast<int64_t>(roadFee)
                      ? curBal - static_cast<int64_t>(roadFee)
                      : 0),
                 std::memory_order_relaxed)) {
      }

      s_cityTreasury.fetch_add(static_cast<int64_t>(roadFee),
                               std::memory_order_relaxed);

      t.speed = 0.0f;
      if (isOverweight) {
        t.state = TruckState::INSPECTION;
        t.stateTimer = 600;
        if ((rand() % 100) < 40) {
          t.state = TruckState::BROKEN_DOWN;
          t.stateTimer = 800;
          g_physicalFeedback[t.id].brokenDown.store(true,
                                                    std::memory_order_release);
          g_physicalFeedback[t.id].pullOverRequested.store(
              true, std::memory_order_release);
        }
      } else {
        t.state = TruckState::INSPECTION;
        t.stateTimer = 300; // 15 seconds (300 ticks)
      }

      Logger::Log(
          "[Logistics] Truck #%u (%s - %s) inspected at Weigh Station #85 (%d "
          "tons, Overweight: %s, Road Fee/Fine: $%u, State: %s)",
          t.id, t.driverName, k_companies[t.companyId].name, t.cargoWeightTons,
          isOverweight ? "YES" : "NO", roadFee,
          t.state == TruckState::BROKEN_DOWN ? "BROKEN_DOWN" : "INSPECTION");
    };

    // 1. Process customer shop/fuel purchase payments from game thread
    EconomyEvent shopEv{};
    while (g_shopPurchaseQueue.pop(shopEv)) {
      if (shopEv.type == EconomyEvent::SHOP_PURCHASE && shopEv.companyId >= 0 &&
          shopEv.companyId < 3) {
        s_companyBalances[shopEv.companyId].fetch_add(
            shopEv.value, std::memory_order_relaxed);
        Logger::Log("[Economy] Company %s received customer revenue: +$%d",
                    k_companies[shopEv.companyId].name, shopEv.value);
      }
    }

    // 2. Track SF port delivery lag for store deficits & decrement strike
    // timers
    s_ticksSinceLastSfDelivery.fetch_add(1, std::memory_order_relaxed);
    for (size_t c = 0; c < 3; ++c) {
      const uint32_t st =
          s_companyStrikeTicks[c].load(std::memory_order_relaxed);
      if (st > 0) {
        s_companyStrikeTicks[c].store(st - 1, std::memory_order_relaxed);
      }
    }

    // 3. Daily Operating Wages, Capital Wealth Tax & Civilized Strikes (every
    // 4000 ticks = ~3.5 minutes)
    if (s_simTickCount % 4000 == 0) {
      const float unrest = s_socialUnrest.load(std::memory_order_relaxed);
      const int64_t dynamicWages =
          1200 + static_cast<int64_t>(unrest *
                                      30.0f); // от $1,200 до $4,200 при бунтах
      for (size_t c = 0; c < 3; ++c) {
        // Налог на свободную ликвидность корпораций (динамическая ставка
        // s_taxCorporateWealth на сверхприбыль > $120,000)
        int64_t bal = s_companyBalances[c].load(std::memory_order_relaxed);
        if (bal > 120000) {
          int64_t excess = bal - 120000;
          const float activeWealthTaxRate =
              s_taxCorporateWealth.load(std::memory_order_relaxed);
          int64_t wealthTax = static_cast<int64_t>(static_cast<float>(excess) *
                                                   activeWealthTaxRate);
          if (wealthTax > 0) {
            s_companyBalances[c].fetch_sub(wealthTax,
                                           std::memory_order_relaxed);
            s_cityTreasury.fetch_add(wealthTax, std::memory_order_relaxed);
            Logger::Log("[Economy] Wealth tax collected from %s (%.1f%%): "
                        "-$%lld (Transferred to City Treasury)",
                        k_companies[c].name, activeWealthTaxRate * 100.0f,
                        static_cast<long long>(wealthTax));
          }
        }

        const int64_t newBal = s_companyBalances[c].fetch_sub(
                                   dynamicWages, std::memory_order_relaxed) -
                               dynamicWages;
        if (newBal < 0) {
          // Balance below $0: Civilized wage strike
          // Trucks do NOT break down in the middle of highway lanes.
          // Instead, set reduced speed (10.0f) and continue towards nearest
          // terminal (Ocean Docks or SF Depot)
          s_companyStrikeTicks[c].store(1600, std::memory_order_relaxed);
          for (size_t i = 0; i < k_totalTruckCount; ++i) {
            if (fleet[i].companyId == c &&
                fleet[i].state == TruckState::EN_ROUTE) {
              fleet[i].speed = 10.0f; // Crawl to nearest terminal
            }
          }
          EconomyEvent strikeEv{.type = EconomyEvent::WAGE_STRIKE,
                                .companyId = static_cast<int>(c),
                                .value = static_cast<int>(newBal),
                                .pos = CVector(0.0f, 0.0f, 0.0f)};
          g_economyEventQueue.push(strikeEv);
          Logger::Log("[Economy] Company %s entered WAGE STRIKE (Balance: "
                      "$%lld, Wages: $%lld) - trucks crawling to terminals at "
                      "10 m/s without blocking highway.",
                      k_companies[c].name, static_cast<long long>(newBal),
                      static_cast<long long>(dynamicWages));
        }
      }
    }

    // 4. Gang Racketeering & Extortion Ambush (scaled dynamically with unrest & crime)
    const bool chipShortageActive =
        s_chipShortageActive.load(std::memory_order_relaxed);
    const float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);
    const float curCrime = s_crimeRate.load(std::memory_order_relaxed);

    uint32_t ambushInterval = 600;
    if (chipShortageActive) {
      ambushInterval = 120;
    } else if (curUnrest >= 60.0f || curCrime >= 60.0f) {
      ambushInterval = 200; // ~10s under heavy unrest & crime
    } else if (curUnrest >= 40.0f || curCrime >= 40.0f) {
      ambushInterval = 400; // ~20s
    }

    if (s_simTickCount % ambushInterval == 0) {
      for (size_t c = 0; c < 3; ++c) {
        const int64_t compBal =
            s_companyBalances[c].load(std::memory_order_relaxed);
        int ambushChance = chipShortageActive ? 40 : 12;
        if (!chipShortageActive) {
          ambushChance = static_cast<int>(12.0f + (curUnrest * 0.35f) + (curCrime * 0.25f));
          ambushChance = std::clamp(ambushChance, 12, 65);
        }
        if ((compBal > 20000 || chipShortageActive) &&
            (rand() % 100) < ambushChance) {
          for (size_t i = 0; i < k_totalTruckCount; ++i) {
            if (fleet[i].companyId == c &&
                fleet[i].state == TruckState::EN_ROUTE) {
              if (chipShortageActive && fleet[i].cargoType != 3) {
                continue; // Target electronics trucks specifically
              }
              int64_t extorted = 1500;
              if (curCrime > 50.0f) {
                extorted = std::max<int64_t>(1500, (compBal * 6) / 100);
              }
              s_companyBalances[c].fetch_sub(extorted,
                                             std::memory_order_relaxed);
              EconomyEvent gangEv{
                  .type = EconomyEvent::GANG_ATTACK,
                  .companyId = static_cast<int>(c),
                  .value = static_cast<int>(extorted),
                  .pos = CVector(fleet[i].x, fleet[i].y, fleet[i].z)};
              g_economyEventQueue.push(gangEv);

              AIDirective hudNotice{.type = AIDirectiveType::DisplayHudNotice,
                                    .truckId = fleet[i].id,
                                    .targetX = fleet[i].x,
                                    .targetY = fleet[i].y,
                                    .targetZ = fleet[i].z,
                                    .heading = fleet[i].heading,
                                    .speed = 0.0f,
                                    .directiveId = ++directiveIdCounter};
              snprintf(
                  hudNotice.noticeMsg, sizeof(hudNotice.noticeMsg),
                  chipShortageActive
                      ? "~r~CHIP HIJACKING: Gangs raided %s Electronics Truck "
                        "#%u! -$%lld~w~"
                      : "~r~EXTORTION: Gangs raided %s Truck #%u! -$%lld~w~",
                  k_companies[c].name, fleet[i].id,
                  static_cast<long long>(extorted));
              g_aiDirectiveQueue.push(hudNotice);

              Logger::Log(
                  "[Economy] Gang ambush on %s Truck #%u (%s)! Extorted $%lld.",
                  k_companies[c].name, fleet[i].id,
                  chipShortageActive ? "CHIP SHORTAGE TARGET" : "General",
                  static_cast<long long>(extorted));
              break;
            }
          }
        }
      }
    }

    // 5. Store Deficits & Ammu-Nation Inflation Multiplier (every 200 ticks =
    // ~10s)
    if (s_simTickCount % 200 == 0) {
      const uint32_t sfDeliveryLag =
          s_ticksSinceLastSfDelivery.load(std::memory_order_relaxed);
      float storeMult =
          1.0f + (static_cast<float>(sfDeliveryLag) / 600.0f) * 0.5f;
      if (storeMult > 3.0f)
        storeMult = 3.0f;
      if (storeMult < 1.0f)
        storeMult = 1.0f;
      s_storePriceMultiplier.store(storeMult, std::memory_order_relaxed);

      EconomyEvent pEv{.type = EconomyEvent::STORE_PRICE_UPDATE,
                       .companyId = 2,
                       .value = static_cast<int>(storeMult * 100.0f),
                       .pos = CVector(0.0f, 0.0f, 0.0f)};
      g_economyEventQueue.push(pEv);
    }

    // 6. Fuel Shortage & Gas Station Price Multiplier (every 200 ticks = ~10s)
    if (s_simTickCount % 200 == 0) {
      size_t delayedFuelTrucks = 0;
      for (size_t i = 0; i < k_totalTruckCount; ++i) {
        if (fleet[i].companyId == 1) {
          if (fleet[i].state == TruckState::BROKEN_DOWN ||
              fleet[i].state == TruckState::INSPECTION ||
              fleet[i].state == TruckState::RESTING) {
            delayedFuelTrucks++;
          }
        }
      }
      const uint32_t currentFuelStock =
          s_sfFuelStock.load(std::memory_order_relaxed);
      float fuelMult = 1.0f + (static_cast<float>(delayedFuelTrucks) * 0.5f) +
                       (currentFuelStock < 30 ? 1.0f : 0.0f);
      if (currentFuelStock < 25 && fuelMult < 2.5f)
        fuelMult = 2.5f;
      fuelMult *= s_activeCrisis.fuelPriceMul.load(std::memory_order_relaxed);
      if (fuelMult > 5.0f)
        fuelMult = 5.0f;
      if (fuelMult < 0.5f)
        fuelMult = 0.5f;
      s_fuelPriceMultiplier.store(fuelMult, std::memory_order_relaxed);

      EconomyEvent fEv{.type = EconomyEvent::FUEL_CRISIS_UPDATE,
                       .companyId = 1,
                       .value = static_cast<int>(fuelMult * 100.0f),
                       .pos = CVector(0.0f, 0.0f, 0.0f)};
      g_economyEventQueue.push(fEv);
    }

    // 7. Player Real Estate Dividends (every 2400 ticks = ~120s)
    if (s_simTickCount % 2400 == 0) {
      int cjOwned = 0;
      for (size_t a = 0; a < 3; ++a) {
        if (s_assetOwners[a].load(std::memory_order_relaxed) == 99) {
          cjOwned++;
        }
      }
      if (cjOwned > 0) {
        const int dividend = cjOwned * 2500;
        EconomyEvent divEv{.type = EconomyEvent::PLAYER_FINANCE,
                           .companyId = 99,
                           .value = dividend,
                           .pos = CVector(0.0f, 0.0f, 0.0f)};
        g_economyEventQueue.push(divEv);
        Logger::Log(
            "[Economy] Paid out $%d logistics real estate dividends to CJ.",
            dividend);
      }
    }

    // Autonomous Real-Estate Bidding:
    // In simulation tick, if an asset is unowned (-1) and any company's balance
    // exceeds its cost: That company buys the asset: deduct cost from
    // s_companyBalances[companyId]. Set s_assetOwners[assetId] = companyId.
    // Queue HUD notice to game screen: "~g~LOGISTICS: %s acquired %s ($%d)~w~"
    // Benefit: All deliveries completed by that company grant +25% payout
    // bonus.
    for (size_t a = 0; a < 3; ++a) {
      if (s_assetOwners[a].load(std::memory_order_relaxed) == -1) {
        for (size_t c = 0; c < 3; ++c) {
          const int64_t bal =
              s_companyBalances[c].load(std::memory_order_relaxed);
          if (bal >= static_cast<int64_t>(k_assets[a].cost)) {
            s_companyBalances[c].fetch_sub(k_assets[a].cost,
                                           std::memory_order_relaxed);
            s_assetOwners[a].store(static_cast<int8_t>(c),
                                   std::memory_order_release);

            AIDirective hudNotice{.type = AIDirectiveType::DisplayHudNotice,
                                  .truckId = 0,
                                  .targetX = 0.0f,
                                  .targetY = 0.0f,
                                  .targetZ = 0.0f,
                                  .heading = 0.0f,
                                  .speed = 0.0f,
                                  .directiveId = ++directiveIdCounter};
            snprintf(hudNotice.noticeMsg, sizeof(hudNotice.noticeMsg),
                     "~g~LOGISTICS: %s acquired %s ($%u)~w~",
                     k_companies[c].name, k_assets[a].name, k_assets[a].cost);
            g_aiDirectiveQueue.push(hudNotice);

            Logger::Log("[Tycoon] %s acquired %s ($%u)", k_companies[c].name,
                        k_assets[a].name, k_assets[a].cost);
            break;
          }
        }
      }
    }

    // Breakdown & Police Inspection Incidents:
    // Checked every 100 ticks = ~5 seconds
    if (s_simTickCount % 100 == 0) {
      for (size_t i = 0; i < k_totalTruckCount; ++i) {
        VirtualTruck &t = fleet[i];
        if (t.state == TruckState::EN_ROUTE) {
          const int64_t bal =
              s_companyBalances[t.companyId].load(std::memory_order_relaxed);
          int threshold = 500;
          if (bal < 5000) {
            threshold = 150;
          } else if (bal < 20000) {
            threshold = 300;
          }

          const bool isOverloaded = (static_cast<float>(t.cargoWeightTons) >
                                     GetCargoDensityCap(t.cargoType));
          t.isOverloaded = isOverloaded;
          if (isOverloaded) {
            threshold = static_cast<int>(static_cast<float>(threshold) * 0.6f);
          }
          if (threshold < 40) {
            threshold = 40;
          }

          if ((rand() % threshold) == 0) {
            t.state = TruckState::BROKEN_DOWN;
            t.speed = 0.0f;
            t.stateTimer = 650;

            const uint32_t towFee = isOverloaded ? 750 : 350;
            int64_t curBal =
                s_companyBalances[t.companyId].load(std::memory_order_relaxed);
            while (curBal > 0 &&
                   !s_companyBalances[t.companyId].compare_exchange_weak(
                       curBal,
                       (curBal >= static_cast<int64_t>(towFee)
                            ? curBal - static_cast<int64_t>(towFee)
                            : 0),
                       std::memory_order_relaxed)) {
            }

            g_physicalFeedback[t.id].brokenDown.store(
                true, std::memory_order_release);
            g_physicalFeedback[t.id].pullOverRequested.store(
                true, std::memory_order_release);
            Logger::Log(
                "[Logistics] Truck #%u (%s - %s) experienced roadside "
                "breakdown! (Overweight: %s, Threshold: %d) -$%u tow fee",
                t.id, t.driverName, k_companies[t.companyId].name,
                isOverloaded ? "YES" : "NO", threshold, towFee);
          } else if ((rand() % 400) == 0) { // 0.25% random police patrol roll
            t.state = TruckState::INSPECTION;
            t.speed = 0.0f;
            t.stateTimer = 300; // 15 seconds (300 ticks)
            g_physicalFeedback[t.id].stopped.store(true,
                                                   std::memory_order_release);
            g_physicalFeedback[t.id].pullOverRequested.store(
                true, std::memory_order_release);
            Logger::Log("[Logistics] Truck #%u (%s - %s) pulled over for "
                        "highway police inspection!",
                        t.id, t.driverName, k_companies[t.companyId].name);
          }
        }
      }
    }

    // Autonomous 20-Store Operational Expenses (Dynamic OpEx / Anti-Bankruptcy
    // Shield) (every 150 seconds = 3000 ticks)
    if (s_simTickCount % 3000 == 0) {
      for (uint32_t i = 0; i < 20; ++i) {
        const int64_t wages = static_cast<int64_t>(
            g_retailStores[i].staffWagesCycle.load(std::memory_order_relaxed));
        const int64_t rent =
            static_cast<int64_t>(g_retailStores[i].propertyRentCycle.load(
                std::memory_order_relaxed));

        const float nominalVel = GetStoreNominalVelocity(
            g_retailStores[i].categoryId, g_retailStores[i].districtId);
        const float curVel = g_retailStores[i].salesVelocityPerMin.load(
            std::memory_order_relaxed);
        const float velocityRatio =
            (nominalVel > 0.001f) ? (curVel / nominalVel) : 1.0f;

        int64_t dynamicWages = wages;
        if (velocityRatio < 0.60f) {
          const float wageScale = (std::max)(0.50f, velocityRatio);
          dynamicWages =
              static_cast<int64_t>(static_cast<float>(wages) * wageScale);

          // Add a slight local unrest penalty (+0.05f) representing worker
          // layoffs, protecting store capital
          const float curU = s_socialUnrest.load(std::memory_order_relaxed);
          const float nextU = (std::min)(100.0f, curU + 0.05f);
          s_socialUnrest.store(nextU, std::memory_order_relaxed);
          s_publicUnrest.store(nextU, std::memory_order_relaxed);
        }

        const int64_t totalOpEx = dynamicWages + rent;
        g_retailStores[i].capitalBalance.fetch_sub(totalOpEx,
                                                   std::memory_order_relaxed);
        s_cityTreasury.fetch_add(rent, std::memory_order_relaxed);
        g_retailStores[i].totalTaxesPaid.fetch_add(static_cast<uint64_t>(rent),
                                                   std::memory_order_relaxed);
      }
    }

    // Fleeca Central Bank, Credit Risk Engine & Bankruptcy State Machine (every
    // 10s = 200 ticks @ 20Hz)
    if (s_simTickCount % 200 == 0) {
      float crime = s_crimeRate.load(std::memory_order_relaxed);
      float unrest = s_socialUnrest.load(std::memory_order_relaxed);
      int64_t reserves =
          g_fleecaBank.totalReserves.load(std::memory_order_relaxed);

      // Base rate = 4.0%
      // Crime impact: +0.06% per crime point
      // Unrest impact: +0.08% per unrest point
      // Liquidity stress: +5.0% if bank reserves drop below $1,000,000
      float rate = 0.04f + (crime * 0.0006f) + (unrest * 0.0008f);
      if (reserves < 1000000) {
        rate += 0.05f;
      }
      // Hard clamp rate between 3.0% (0.03f) and 22.0% (0.22f)
      if (rate < 0.03f)
        rate = 0.03f;
      if (rate > 0.22f)
        rate = 0.22f;
      g_fleecaBank.dynamicInterestRate.store(rate, std::memory_order_relaxed);

      for (uint32_t i = 0; i < 20; ++i) {
        auto &store = g_retailStores[i];

        // 1. If isBankrupt == true:
        if (store.isBankrupt.load(std::memory_order_relaxed)) {
          // Check City Hall Bailout condition:
          if (store.districtId == 1) { // Downtown / Wealthy
            if (s_cityTreasury.load(std::memory_order_relaxed) >= 50000) {
              s_cityTreasury.fetch_sub(50000, std::memory_order_relaxed);
              store.loanDebt.store(0, std::memory_order_relaxed);
              store.capitalBalance.store(10000, std::memory_order_relaxed);
              store.creditScore.store(50, std::memory_order_relaxed);
              store.isBankrupt.store(false, std::memory_order_relaxed);
              g_fleecaBank.activeBailoutsCount.fetch_add(
                  1, std::memory_order_relaxed);
              if (g_fleecaBank.bankruptStoresCount.load(
                      std::memory_order_relaxed) > 0) {
                g_fleecaBank.bankruptStoresCount.fetch_sub(
                    1, std::memory_order_relaxed);
              }
            }
          }
          continue;
        }

        // 2. If loanDebt > 0:
        int64_t curDebt = store.loanDebt.load(std::memory_order_relaxed);
        if (curDebt > 0) {
          // Accrue interest:
          int64_t interestCharge =
              static_cast<int64_t>(curDebt * (rate / 12.0f));
          store.loanDebt.fetch_add(interestCharge, std::memory_order_relaxed);

          // Auto-Repayment Logic:
          int64_t capBal = store.capitalBalance.load(std::memory_order_relaxed);
          if (capBal > 12000) {
            int64_t surplus = capBal - 12000;
            int64_t latestDebt = store.loanDebt.load(std::memory_order_relaxed);
            int64_t repayment = std::min(latestDebt, surplus / 2);
            if (repayment > 0) {
              store.capitalBalance.fetch_sub(repayment,
                                             std::memory_order_relaxed);
              store.loanDebt.fetch_sub(repayment, std::memory_order_relaxed);
              g_fleecaBank.totalReserves.fetch_add(repayment,
                                                   std::memory_order_relaxed);
              int32_t curScore =
                  store.creditScore.load(std::memory_order_relaxed);
              store.creditScore.store(std::min(100, curScore + 2),
                                      std::memory_order_relaxed);
            }
          }
        }

        // 3. Deficit Handling (capitalBalance < 0):
        int64_t curCapBal =
            store.capitalBalance.load(std::memory_order_relaxed);
        if (curCapBal < 0) {
          int32_t cScore = store.creditScore.load(std::memory_order_relaxed);
          int64_t lDebt = store.loanDebt.load(std::memory_order_relaxed);
          int64_t bReserves =
              g_fleecaBank.totalReserves.load(std::memory_order_relaxed);

          if (cScore >= 35 && lDebt < 75000 && bReserves >= 10000) {
            // Eligible for emergency tranche:
            int64_t deficit = -curCapBal;
            int64_t tranche =
                deficit + 4000; // Cover debt and give $4000 buffer
            store.capitalBalance.store(4000, std::memory_order_relaxed);
            store.loanDebt.fetch_add(tranche, std::memory_order_relaxed);
            g_fleecaBank.totalReserves.fetch_sub(tranche,
                                                 std::memory_order_relaxed);
            store.creditScore.store(std::max(0, cScore - 8),
                                    std::memory_order_relaxed);
          } else {
            // Default / Bankruptcy:
            store.isBankrupt.store(true, std::memory_order_relaxed);
            g_fleecaBank.bankruptStoresCount.fetch_add(
                1, std::memory_order_relaxed);
            // Shock local district unrest:
            s_socialUnrest.store(
                std::min(100.0f,
                         s_socialUnrest.load(std::memory_order_relaxed) + 3.5f),
                std::memory_order_relaxed);
          }
        }

        // 4. Organic Score Recovery:
        if (store.capitalBalance.load(std::memory_order_relaxed) > 5000 &&
            store.loanDebt.load(std::memory_order_relaxed) == 0 &&
            store.creditScore.load(std::memory_order_relaxed) < 100) {
          int32_t curScore = store.creditScore.load(std::memory_order_relaxed);
          store.creditScore.store(std::min(100, curScore + 1),
                                  std::memory_order_relaxed);
        }
      }
    }

    // =====================================================================
    //  District-Driven Demand Matrix & Dynamic Consumption Engine (per-tick dt)
    // =====================================================================
    EconomyRetail::UpdateRetailStores(static_cast<uint32_t>(dt * 1000.0f));

    // Retail Sales Velocity Window Reset & District Citizen Poll Tax (Every
    // 1200 ticks = ~60 seconds @ 20Hz)
    if (s_simTickCount % 1200 == 0) {
      for (uint32_t i = 0; i < 20; ++i) {
        g_retailStores[i].windowUnitsSold.store(0, std::memory_order_relaxed);
      }

      // Citizen Poll Tax & Social Unrest Cycle:
      // Sum citizen tax revenue across all 4 districts weighted by census
      // population density
      static const float k_districtPopulationWeight[4] = {1.20f, 1.00f, 0.80f,
                                                          0.50f};
      int64_t totalDistrictTax = 0;
      for (uint32_t d = 0; d < 4; ++d) {
        const float localPollTax =
            s_districtPollTax[d].load(std::memory_order_relaxed);
        if (localPollTax > 0.0f) {
          totalDistrictTax += static_cast<int64_t>(
              localPollTax * k_districtPopulationWeight[d]);
        }
      }
      if (totalDistrictTax > 0) {
        s_cityTreasury.fetch_add(totalDistrictTax, std::memory_order_relaxed);
        Logger::Log("[MunicipalTax] Collected $%lld in citizen poll taxes "
                    "across 4 district zones",
                    static_cast<long long>(totalDistrictTax));
      }

      // Evaluate complete living standards, basket deduction, and inertial
      // slew-rate limited unrest/crime
      EvaluateDistrictLivingStandards(true);
    }

    // Supply Starvation Municipal Loop: Evaluate essential stock once every 180
    // seconds (3600 ticks @ 20Hz)
    if (s_simTickCount % 3600 == 0) {
      uint32_t districtStock[4] = {0, 0, 0, 0};
      uint32_t districtCap[4] = {0, 0, 0, 0};
      for (uint32_t i = 0; i < 20; ++i) {
        const uint32_t catId = g_retailStores[i].categoryId;
        if (catId == 0 || catId == 1 ||
            catId == 2) { // Essential: Food, Fuel, & Agro Produce
          const uint32_t dId = g_retailStores[i].districtId;
          if (dId < 4) {
            districtStock[dId] +=
                g_retailStores[i].localStock.load(std::memory_order_relaxed);
            districtCap[dId] +=
                g_retailStores[i].maxCapacity.load(std::memory_order_relaxed);
          }
        }
      }

      const char *const districtNames[4] = {
          "South Central", "Downtown", "Industrial Port", "Country / Highway"};
      for (uint32_t d = 0; d < 4; ++d) {
        if (districtCap[d] > 0) {
          const float ratio = static_cast<float>(districtStock[d]) /
                              static_cast<float>(districtCap[d]);
          // Only apply penalty if essential stock drops below 10%, max +0.05f
          // unrest
          if (ratio < 0.10f) {
            float curU = s_socialUnrest.load(std::memory_order_relaxed);
            s_socialUnrest.store((std::min)(100.0f, curU + 0.05f),
                                 std::memory_order_relaxed);
            s_publicUnrest.store(s_socialUnrest.load(std::memory_order_relaxed),
                                 std::memory_order_relaxed);
            AddMunicipalLog("SUPPLY STARVATION: District %s essential stock "
                            "critical <10%% (%.1f%%). Unrest rising (+0.05%%).",
                            districtNames[d], ratio * 100.0f);
            Logger::Log("[Municipal] Supply Starvation Alert: District %s "
                        "essential stock critical (%.1f%%)",
                        districtNames[d], ratio * 100.0f);
          } else if (ratio > 0.60f) {
            float curU = s_socialUnrest.load(std::memory_order_relaxed);
            if (curU > 5.0f) {
              s_socialUnrest.store((std::max)(5.0f, curU - 0.05f),
                                   std::memory_order_relaxed);
              s_publicUnrest.store(
                  s_socialUnrest.load(std::memory_order_relaxed),
                  std::memory_order_relaxed);
            }
          }
        }
      }
    }

    // Periodic Telemetry Buffer Serialization (Every 20 ticks = 1 second @
    // 20Hz)
    if (s_simTickCount % 20 == 0) {
      EconomyTelemetry::SerializeTelemetryJson();
    }

    const float playerX = g_isPlayerValid.load(std::memory_order_relaxed)
                              ? g_playerPosX.load(std::memory_order_relaxed)
                              : g_playerX.load(std::memory_order_relaxed);
    const float playerY = g_isPlayerValid.load(std::memory_order_relaxed)
                              ? g_playerPosY.load(std::memory_order_relaxed)
                              : g_playerY.load(std::memory_order_relaxed);

    for (size_t i = 0; i < k_totalTruckCount; ++i) {
      VirtualTruck &truck = fleet[i];

      // Synchronize truck.isMaterialized directly with the real engine state
      const bool isPhysical =
          g_physicalFeedback[truck.id].active.load(std::memory_order_acquire);
      truck.isMaterialized = isPhysical;

      // Vehicle Destruction Detection & Financial Penalty
      if (g_physicalFeedback[truck.id].destroyed.load(
              std::memory_order_acquire) &&
          truck.state != TruckState::DESTROYED) {
        truck.state = TruckState::DESTROYED;
        truck.speed = 0.0f;
        const int64_t curBal =
            s_companyBalances[truck.companyId].load(std::memory_order_relaxed);
        if (curBal >= 5000) {
          s_companyBalances[truck.companyId].fetch_sub(
              5000, std::memory_order_relaxed);
        } else {
          s_companyBalances[truck.companyId].store(0,
                                                   std::memory_order_relaxed);
        }
        truck.stateTimer = 600; // ~30 seconds wreck clearance timeout
        Logger::Log("[Logistics] Truck #%u (%s - %s) DESTROYED! -$5000 "
                    "write-off insurance penalty",
                    truck.id, truck.driverName,
                    k_companies[truck.companyId].name);
      }

      // Autonomous Roadside Emergency Refuel: If fuel <= 5.0%
      if (truck.fuel <= 5.0f) {
        truck.fuel = 100.0f;
        const int64_t curBal =
            s_companyBalances[truck.companyId].load(std::memory_order_relaxed);
        if (curBal >= 250) {
          s_companyBalances[truck.companyId].fetch_sub(
              250, std::memory_order_relaxed);
        }
      }

      // Autonomous Roadside Fatigue Break: If fatigue >= 95.0% and EN_ROUTE
      if (truck.fatigue >= 95.0f && truck.state == TruckState::EN_ROUTE) {
        truck.state = TruckState::RESTING;
        truck.stateTimer = 500; // 25 seconds (500 ticks)
        truck.speed = 0.0f;
        g_physicalFeedback[truck.id].pullOverRequested.store(
            true, std::memory_order_release);
        const int64_t curBal =
            s_companyBalances[truck.companyId].load(std::memory_order_relaxed);
        if (curBal >= 50) {
          s_companyBalances[truck.companyId].fetch_sub(
              50, std::memory_order_relaxed);
        }
      }

      // -----------------------------------------------------------------------
      // Section 1: Digital Police Clearance & Roadblock Interconnect (<= 45.0m)
      // -----------------------------------------------------------------------
      const uint32_t currentMs = CTimer::m_snTimeInMilliseconds != 0
          ? CTimer::m_snTimeInMilliseconds
          : static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

      if (truck.state == TruckState::EN_ROUTE || truck.state == TruckState::RESTING) {
        int activeCpIdx = -1;
        const uint64_t activeRbMask = s_activeRoadblockMask.load(std::memory_order_acquire);
        for (size_t cp = 0; cp < k_numArterialChokepoints; ++cp) {
          const auto cpStatus = s_arterialChokepoints[cp].status.load(std::memory_order_relaxed);
          const bool isActive = ((activeRbMask & (1ULL << cp)) != 0) || (cpStatus != ChokepointStatus::CLEAR);
          if (!isActive) continue;

          const float dxCp = k_chokepointsTable[cp].x - truck.x;
          const float dyCp = k_chokepointsTable[cp].y - truck.y;
          const float dzCp = k_chokepointsTable[cp].z - truck.z;
          if ((dxCp * dxCp + dyCp * dyCp + dzCp * dzCp) <= (45.0f * 45.0f)) {
            activeCpIdx = static_cast<int>(cp);
            break;
          }
        }

        if (activeCpIdx >= 0) {
          const size_t cpIdx = static_cast<size_t>(activeCpIdx);
          auto &cp = s_arterialChokepoints[cpIdx];

          // Truck stops (sets zero velocity, stays in TruckState::RESTING)
          truck.speed = 0.0f;
          truck.state = TruckState::RESTING;
          g_physicalFeedback[truck.id].stopped.store(true, std::memory_order_release);

          const auto cpStatus = cp.status.load(std::memory_order_relaxed);
          if (cpStatus == ChokepointStatus::BLOCKED_RED) {
            const float desertion = s_policeDesertionPct.load(std::memory_order_relaxed);
            const float unrest = s_publicUnrest.load(std::memory_order_relaxed);
            const bool hasFuel = (s_cityTreasury.load(std::memory_order_relaxed) > 5000);

            if (desertion < 50.0f && hasFuel && unrest < 90.0f) {
              cp.status.store(ChokepointStatus::CLEARING_BLUE_BLINK, std::memory_order_release);
              cp.clearTimerMs = currentMs + 25000;
              AddMunicipalLog("[PoliceDispatch] Tactical escort arriving at chokepoint #%zu. Breaching barricade...", cpIdx);
            } else {
              // Police starved/unavailable. Road stays BLOCKED_RED.
              // Truck pays demurrage:
              if (truck.blockedSinceMs == 0 || (currentMs - truck.blockedSinceMs >= 5000)) {
                truck.blockedSinceMs = currentMs;
                DeductCompanyBalance(truck.companyId, 500, "Roadblock extortion/demurrage");
              }
            }
            continue;
          } else if (cpStatus == ChokepointStatus::CLEARING_BLUE_BLINK) {
            if (currentMs >= cp.clearTimerMs) {
              cp.status.store(ChokepointStatus::CLEAR, std::memory_order_release);
              s_activeRoadblockMask.fetch_and(~(1ULL << cpIdx), std::memory_order_release);
              truck.state = TruckState::IN_TRANSIT;
              truck.blockedSinceMs = 0;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(false, std::memory_order_release);
              AddMunicipalLog("[PoliceDispatch] Barricade breached at chokepoint #%zu. Transit corridor opened.", cpIdx);
            } else {
              // While clearing, truck remains stopped in TruckState::RESTING
              continue;
            }
          }
        } else if (truck.blockedSinceMs != 0) {
          // Roadblock cleared elsewhere
          truck.state = TruckState::IN_TRANSIT;
          truck.blockedSinceMs = 0;
          truck.speed = 22.0f;
          g_physicalFeedback[truck.id].stopped.store(false, std::memory_order_release);
        }
      }

      // Contract Deadline Tracking: 28-minute deadline (33600 ticks) upon
      // leaving Ocean Docks
      if (truck.cargoType != 0 && truck.state == TruckState::EN_ROUTE) {
        if (truck.deadlineTicks > 0) {
          --truck.deadlineTicks;
        } else if (!truck.deadlinePenalized) {
          truck.deadlinePenalized = true;
          const uint32_t deadlineFee = static_cast<uint32_t>(
              static_cast<float>(truck.cargoWeightTons) * 120.0f + 1200.0f);
          int64_t curBal = s_companyBalances[truck.companyId].load(
              std::memory_order_relaxed);
          while (curBal > 0 &&
                 !s_companyBalances[truck.companyId].compare_exchange_weak(
                     curBal,
                     (curBal >= static_cast<int64_t>(deadlineFee)
                          ? curBal - static_cast<int64_t>(deadlineFee)
                          : 0),
                     std::memory_order_relaxed)) {
          }
          Logger::Log("[Logistics] Truck #%u (%s - %s) missed 28-minute "
                      "delivery deadline! -$%u late penalty",
                      truck.id, truck.driverName,
                      k_companies[truck.companyId].name, deadlineFee);
        }
      }

      // Civilized Wage Strike: slow crawl to nearest terminal without blocking
      // highway lanes
      const bool inWageStrike = (s_companyBalances[truck.companyId].load(
                                     std::memory_order_relaxed) < 0) ||
                                (s_companyStrikeTicks[truck.companyId].load(
                                     std::memory_order_relaxed) > 0);
      if (inWageStrike && truck.state == TruckState::EN_ROUTE) {
        truck.speed = 10.0f; // Crawl to nearest terminal
      } else if (truck.isOverloaded && truck.state == TruckState::EN_ROUTE) {
        truck.speed = 12.0f; // Overload speed throttle
      }

      // Sync feedback properties to lock-free atomic registers
      g_physicalFeedback[truck.id].cargoWeightTons.store(
          static_cast<uint32_t>(truck.cargoWeightTons),
          std::memory_order_relaxed);
      g_physicalFeedback[truck.id].companyId.store(truck.companyId,
                                                   std::memory_order_relaxed);
      g_physicalFeedback[truck.id].isOverloaded.store(
          truck.isOverloaded, std::memory_order_relaxed);
      g_physicalFeedback[truck.id].cargoType.store(
          static_cast<uint8_t>(truck.cargoType), std::memory_order_relaxed);

      // Handle truck hijack delivery reset back to Ocean Docks
      if (g_physicalFeedback[truck.id].hijackedReset.load(
              std::memory_order_acquire)) {
        g_physicalFeedback[truck.id].hijackedReset.store(
            false, std::memory_order_release);
        truck.currentTargetNode = 0;
        truck.travelForward = true;
        g_physicalFeedback[truck.id].targetWaypointIndex.store(
            0, std::memory_order_release);
        g_physicalFeedback[truck.id].travelForward.store(
            true, std::memory_order_release);
        const HighwayWaypoint &resetWp = s_highwayLoop[0];
        truck.x = resetWp.x;
        truck.y = resetWp.y;
        truck.z = resetWp.z;
        truck.fuel = 100.0f;
        truck.fatigue = 0.0f;
        if (truck.companyId == 2) {
          truck.cargoType = 4;
          truck.cargoWeightTons = 16 + (static_cast<int>(truck.id) % 7);
        } else if (truck.companyId == 1) {
          truck.cargoType = 2;
          truck.cargoWeightTons = 20 + (static_cast<int>(truck.id) % 7);
        } else {
          truck.cargoType = 1;
          truck.cargoWeightTons = 14 + (static_cast<int>(truck.id) % 5);
        }
        truck.state = TruckState::LOADING;
        truck.stateTimer = 1600; // ~80 seconds
        truck.speed = 0.0f;
        ApplyCargoComplianceAndLoading(truck);
        if (isPhysical) {
          const AIDirective despawnDir{.type = AIDirectiveType::DespawnTruck,
                                       .truckId = truck.id,
                                       .targetX = 0.0f,
                                       .targetY = 0.0f,
                                       .targetZ = 0.0f,
                                       .heading = 0.0f,
                                       .speed = 0.0f,
                                       .directiveId = ++directiveIdCounter};
          g_aiDirectiveQueue.push(despawnDir);
        }
      }

      // If materialized in 3D world, sync coordinates from game thread feedback
      if (truck.isMaterialized) {
        truck.x =
            g_physicalFeedback[truck.id].x.load(std::memory_order_relaxed);
        truck.y =
            g_physicalFeedback[truck.id].y.load(std::memory_order_relaxed);
        truck.z =
            g_physicalFeedback[truck.id].z.load(std::memory_order_relaxed);
        truck.heading = g_physicalFeedback[truck.id].heading.load(
            std::memory_order_relaxed);

        // Sync waypoint progress from game thread
        const uint32_t liveWp =
            g_physicalFeedback[truck.id].targetWaypointIndex.load(
                std::memory_order_acquire);
        if (liveWp != truck.currentTargetNode) {
          if (truck.currentTargetNode == 0) {
            handleOceanDocksArrival(truck);
          } else if (truck.currentTargetNode == 85) {
            handleWeighStationInspection(truck);
            g_physicalFeedback[truck.id].stopped.store(
                true, std::memory_order_release);
            g_physicalFeedback[truck.id].pullOverRequested.store(
                true, std::memory_order_release);
          } else if (truck.currentTargetNode == 135) {
            truck.state = TruckState::RESTING;
            truck.stateTimer = 500; // 25 seconds (500 ticks)
            truck.speed = 0.0f;
            g_physicalFeedback[truck.id].pullOverRequested.store(
                true, std::memory_order_release);
          } else if (truck.currentTargetNode == 240) {
            truck.state = TruckState::UNLOADING;
            truck.stateTimer = 1600; // ~80 seconds (1600 ticks)
            truck.speed = 0.0f;
          }
          truck.currentTargetNode = liveWp % s_waypointCount;
        }

        if (truck.state != TruckState::EN_ROUTE) {
          g_physicalFeedback[truck.id].stopped.store(true,
                                                     std::memory_order_release);
          truck.speed = 0.0f;
          if (truck.state == TruckState::RESTING) {
            truck.fatigue = std::max(0.0f, truck.fatigue - 1.5f);
          }
          if (truck.stateTimer > 0) {
            --truck.stateTimer;
          }
          if (truck.stateTimer == 0) {
            if (truck.state == TruckState::RESTING) {
              if (truck.blockedSinceMs != 0) {
                // Maintained by roadblock digital clearance
                truck.stateTimer = 20;
              } else {
                truck.fatigue = 0.0f;
                truck.state = TruckState::EN_ROUTE;
                truck.speed = 22.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              }
            } else if (truck.state == TruckState::UNLOADING) {
              if (truck.id >= k_truckCount) {
                // Feeder Truck finished 6.0s unloading at Port Terminus
                const uint32_t grossFee = 12500;
                // Progressive municipal harbor tariff: 30% routed to city treasury
                const uint32_t portTariff = (grossFee * 30) / 100;
                s_cityTreasury.fetch_add(static_cast<int64_t>(portTariff), std::memory_order_relaxed);
                s_companyBalances[truck.companyId].fetch_add(
                    grossFee - portTariff, std::memory_order_relaxed);
                truck.driverWallet += 400;
                truck.deliveriesDone++;
                truck.experience += 100;
                if (truck.experience >= 800)
                  truck.skillLevel = 3;
                else if (truck.experience >= 300)
                  truck.skillLevel = 2;

                // Hard ceiling clamp: do not accept cargo beyond maxCapacity
                std::atomic<uint32_t>* targetPortStock = nullptr;
                const char* cargoName = "Freight";
                if (truck.companyId == 2) {
                  targetPortStock = &s_portFoodStock;
                  cargoName = "Food";
                } else if (truck.companyId == 1) {
                  targetPortStock = &s_portFuelStock;
                  cargoName = "Fuel";
                } else {
                  targetPortStock = &s_sfElectronicsStock;
                  cargoName = "Freight";
                }

                const uint32_t current = targetPortStock->load(std::memory_order_relaxed);
                const uint32_t maxCap = 500;
                const uint32_t cargoTons = truck.cargoWeightTons * 4;
                if (current < maxCap) {
                  const uint32_t availableSpace = maxCap - current;
                  const uint32_t actualDelivered = (cargoTons > availableSpace) ? availableSpace : cargoTons;
                  targetPortStock->fetch_add(actualDelivered, std::memory_order_relaxed);
                }
                Logger::Log("[Feeder] Truck #%u deposited %d tons %s at Port Terminus (+%u company, +%u tariff, Stock: %u/%u)",
                            truck.id, truck.cargoWeightTons, cargoName, grossFee - portTariff, portTariff,
                            targetPortStock->load(std::memory_order_relaxed), maxCap);
                truck.cargoWeightTons = 0;
                truck.cargoType = 0;

                const auto &rNodes = g_customRoutes[truck.companyId].nodes;
                truck.state = TruckState::EN_ROUTE;
                truck.speed = 22.0f;
                truck.travelForward = false;
                truck.currentTargetNode =
                    (rNodes.size() > 1) ? rNodes.size() - 2 : 0;
                g_physicalFeedback[truck.id].travelForward.store(
                    false, std::memory_order_release);
                g_physicalFeedback[truck.id].targetWaypointIndex.store(
                    static_cast<uint32_t>(truck.currentTargetNode),
                    std::memory_order_release);
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              } else {
                handleSfUnload(truck);
                if (s_companyBalances[truck.companyId].load(
                        std::memory_order_relaxed) < 0) {
                  truck.stateTimer = 1600;
                  truck.speed = 0.0f;
                  g_physicalFeedback[truck.id].stopped.store(
                      true, std::memory_order_release);
                } else {
                  truck.state = TruckState::EN_ROUTE;
                  truck.speed = 22.0f;
                  g_physicalFeedback[truck.id].stopped.store(
                      false, std::memory_order_release);
                }
              }
            } else if (truck.state == TruckState::LOADING) {
              if (truck.id >= k_truckCount) {
                // Dispatch check: trucks should not target hubs that are >= 95% full
                std::atomic<uint32_t>* targetPortStock = nullptr;
                if (truck.companyId == 2) targetPortStock = &s_portFoodStock;
                else if (truck.companyId == 1) targetPortStock = &s_portFuelStock;
                else targetPortStock = &s_sfElectronicsStock;

                const uint32_t currentPortStock = targetPortStock->load(std::memory_order_relaxed);
                constexpr uint32_t portMaxCap = 500;
                if (currentPortStock >= (portMaxCap * 95) / 100) {
                  // Hub >= 95% full: hold truck at factory loading bay
                  truck.stateTimer = 400; // wait ~20s before re-checking
                  truck.speed = 0.0f;
                  g_physicalFeedback[truck.id].stopped.store(true, std::memory_order_release);
                  continue;
                }

                // Feeder Truck finished 6.0s loading at Node 0 Factory Hub
                if (truck.companyId == 2) {
                  truck.cargoType = 4; // Fresh Agricultural Produce / Grain
                  truck.cargoWeightTons = 25;
                  Logger::Log("[Feeder] Truck #%u finished loading 25 tons "
                              "Food at Agro Hub (Node 0)",
                              truck.id);
                } else if (truck.companyId == 1) {
                  truck.cargoType = 2; // Petrochemical / High-Octane Fuel
                  truck.cargoWeightTons = 25;
                  Logger::Log("[Feeder] Truck #%u finished loading 25 tons "
                              "Fuel at Refinery Hub (Node 0)",
                              truck.id);
                } else {
                  truck.cargoType = 3; // Intermodal Freight / Containers
                  truck.cargoWeightTons = 20;
                  Logger::Log("[Feeder] Truck #%u finished loading 20 tons "
                              "Freight at Freight Hub (Node 0)",
                              truck.id);
                }
                ApplyCargoComplianceAndLoading(truck);

                const auto &rNodes = g_customRoutes[truck.companyId].nodes;
                truck.state = TruckState::EN_ROUTE;
                truck.speed = 22.0f;
                truck.travelForward = true;
                truck.currentTargetNode = (rNodes.size() > 1) ? 1 : 0;
                g_physicalFeedback[truck.id].travelForward.store(
                    true, std::memory_order_release);
                g_physicalFeedback[truck.id].targetWaypointIndex.store(
                    static_cast<uint32_t>(truck.currentTargetNode),
                    std::memory_order_release);
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              } else {
                if (s_companyBalances[truck.companyId].load(
                        std::memory_order_relaxed) < 0) {
                  truck.stateTimer = 1600;
                  truck.speed = 0.0f;
                  g_physicalFeedback[truck.id].stopped.store(
                      true, std::memory_order_release);
                } else {
                  truck.state = TruckState::EN_ROUTE;
                  truck.speed = 22.0f;
                  truck.deadlineTicks =
                      33600; // 28 minutes deadline upon leaving Ocean Docks
                  truck.deadlinePenalized = false;
                  g_physicalFeedback[truck.id].stopped.store(
                      false, std::memory_order_release);
                }
              }
            } else if (truck.state == TruckState::INSPECTION) {
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            } else if (truck.state == TruckState::BROKEN_DOWN) {
              g_physicalFeedback[truck.id].brokenDown.store(
                  false, std::memory_order_release);
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            } else if (truck.state == TruckState::DESTROYED) {
              g_physicalFeedback[truck.id].destroyed.store(
                  false, std::memory_order_release);
              truck.currentTargetNode = 0;
              truck.travelForward = true;
              g_physicalFeedback[truck.id].targetWaypointIndex.store(
                  0, std::memory_order_release);
              g_physicalFeedback[truck.id].travelForward.store(
                  true, std::memory_order_release);
              const HighwayWaypoint &resetWp = s_highwayLoop[0];
              truck.x = resetWp.x;
              truck.y = resetWp.y;
              truck.z = resetWp.z;
              truck.fuel = 100.0f;
              truck.fatigue = 0.0f;
              if (truck.companyId == 2) {
                truck.cargoType = 4;
                truck.cargoWeightTons = 16 + (static_cast<int>(truck.id) % 7);
              } else if (truck.companyId == 1) {
                truck.cargoType = 2;
                truck.cargoWeightTons = 20 + (static_cast<int>(truck.id) % 7);
              } else {
                truck.cargoType = 1;
                truck.cargoWeightTons = 14 + (static_cast<int>(truck.id) % 5);
              }
              ApplyCargoComplianceAndLoading(truck);
              truck.state = TruckState::LOADING;
              truck.stateTimer = 1600; // ~80 seconds
              truck.speed = 0.0f;
            } else {
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            }
          }
        } else {
          g_physicalFeedback[truck.id].stopped.store(false,
                                                     std::memory_order_release);
          const float fuelBurn =
              (truck.skillLevel >= 3) ? (0.004f * 0.7f) : 0.004f;
          const float fatigueGain =
              (truck.skillLevel >= 3) ? (0.0035f * 0.7f) : 0.0035f;
          truck.fuel = std::max(0.0f, truck.fuel - fuelBurn);
          truck.fatigue = std::min(100.0f, truck.fatigue + fatigueGain);

          if (truck.id < k_truckCount) {
            // Keep currentTargetNode updated while physical along master
            // highway loop
            const HighwayWaypoint &targetWp =
                s_highwayLoop[truck.currentTargetNode % s_waypointCount];
            const float dx = targetWp.x - truck.x;
            const float dy = targetWp.y - truck.y;
            if ((dx * dx + dy * dy) < (18.0f * 18.0f)) {
              const size_t nextNode =
                  (truck.currentTargetNode + 1) % s_waypointCount;
              g_physicalFeedback[truck.id].targetWaypointIndex.store(
                  static_cast<uint32_t>(nextNode), std::memory_order_release);
              if (truck.currentTargetNode == 0) {
                handleOceanDocksArrival(truck);
              } else if (truck.currentTargetNode == 85) {
                handleWeighStationInspection(truck);
                g_physicalFeedback[truck.id].stopped.store(
                    true, std::memory_order_release);
                g_physicalFeedback[truck.id].pullOverRequested.store(
                    true, std::memory_order_release);
              } else if (truck.currentTargetNode == 135) {
                truck.state = TruckState::RESTING;
                truck.stateTimer = 500; // 25 seconds (500 ticks)
                truck.speed = 0.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              } else if (truck.currentTargetNode == 240) {
                truck.state = TruckState::UNLOADING;
                truck.stateTimer = 1600; // ~80 seconds (1600 ticks)
                truck.speed = 0.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              }
              truck.currentTargetNode = nextNode;
            }
          } else {
            // Feeder truck physical waypoint check
            const auto &rNodes = g_customRoutes[truck.companyId].nodes;
            if (rNodes.size() >= 2) {
              const auto &termNode = rNodes[rNodes.size() - 1];
              const float distTerm =
                  std::hypot(truck.x - termNode.x, truck.y - termNode.y);
              const float distPort =
                  std::hypot(truck.x - 2312.2f, truck.y - (-2252.0f));
              const auto &originNode = rNodes[0];
              const float distOrigin =
                  std::hypot(truck.x - originNode.x, truck.y - originNode.y);

              if ((distTerm < 15.0f || distPort < 15.0f) &&
                  truck.cargoWeightTons > 0 && truck.travelForward) {
                truck.state = TruckState::UNLOADING;
                truck.stateTimer = 120; // 6.0 seconds (120 ticks) hold position
                truck.speed = 0.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    true, std::memory_order_release);
              } else if (distOrigin < 15.0f && truck.cargoWeightTons == 0 &&
                         !truck.travelForward) {
                truck.state = TruckState::LOADING;
                truck.stateTimer = 120; // 6.0 seconds (120 ticks) hold position
                truck.speed = 0.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    true, std::memory_order_release);
              } else {
                size_t curIdx = truck.currentTargetNode;
                if (curIdx >= rNodes.size())
                  curIdx = rNodes.size() - 1;
                const auto &targetNode = rNodes[curIdx];
                const float dx = targetNode.x - truck.x;
                const float dy = targetNode.y - truck.y;
                if ((dx * dx + dy * dy) < (18.0f * 18.0f)) {
                  if (truck.travelForward) {
                    if (curIdx + 1 < rNodes.size()) {
                      truck.currentTargetNode = curIdx + 1;
                    } else {
                      if (truck.cargoWeightTons > 0) {
                        truck.state = TruckState::UNLOADING;
                        truck.stateTimer = 120; // 6.0 seconds
                        truck.speed = 0.0f;
                        g_physicalFeedback[truck.id].stopped.store(
                            true, std::memory_order_release);
                      }
                    }
                  } else {
                    if (curIdx > 0) {
                      truck.currentTargetNode = curIdx - 1;
                    } else {
                      if (truck.cargoWeightTons == 0) {
                        truck.state = TruckState::LOADING;
                        truck.stateTimer = 120; // 6.0 seconds
                        truck.speed = 0.0f;
                        g_physicalFeedback[truck.id].stopped.store(
                            true, std::memory_order_release);
                      }
                    }
                  }
                  g_physicalFeedback[truck.id].targetWaypointIndex.store(
                      static_cast<uint32_t>(truck.currentTargetNode),
                      std::memory_order_release);
                }
              }
            }
          }
        }
      } else {
        // Advance unmaterialized virtual truck along authentic highway loop
        // towards next highway node
        if (truck.state != TruckState::EN_ROUTE) {
          g_physicalFeedback[truck.id].stopped.store(true,
                                                     std::memory_order_release);
          truck.speed = 0.0f;
          if (truck.state == TruckState::RESTING) {
            truck.fatigue = std::max(0.0f, truck.fatigue - 1.5f);
          }
          if (truck.stateTimer > 0) {
            --truck.stateTimer;
          }
          if (truck.stateTimer == 0) {
            if (truck.state == TruckState::RESTING) {
              truck.fatigue = 0.0f;
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            } else if (truck.state == TruckState::UNLOADING) {
              if (truck.id >= k_truckCount) {
                // Feeder Truck finished 6.0s unloading at Port Terminus
                const uint32_t grossFee = 12500;
                // Progressive municipal harbor tariff: 30% routed to city treasury
                const uint32_t portTariff = (grossFee * 30) / 100;
                s_cityTreasury.fetch_add(static_cast<int64_t>(portTariff), std::memory_order_relaxed);
                s_companyBalances[truck.companyId].fetch_add(
                    grossFee - portTariff, std::memory_order_relaxed);
                truck.driverWallet += 400;
                truck.deliveriesDone++;
                truck.experience += 100;
                if (truck.experience >= 800)
                  truck.skillLevel = 3;
                else if (truck.experience >= 300)
                  truck.skillLevel = 2;

                // Hard ceiling clamp: do not accept cargo beyond maxCapacity
                std::atomic<uint32_t>* targetPortStock = nullptr;
                const char* cargoName = "Freight";
                if (truck.companyId == 2) {
                  targetPortStock = &s_portFoodStock;
                  cargoName = "Food";
                } else if (truck.companyId == 1) {
                  targetPortStock = &s_portFuelStock;
                  cargoName = "Fuel";
                } else {
                  targetPortStock = &s_sfElectronicsStock;
                  cargoName = "Freight";
                }

                const uint32_t current = targetPortStock->load(std::memory_order_relaxed);
                const uint32_t maxCap = 500;
                const uint32_t cargoTons = truck.cargoWeightTons * 4;
                if (current < maxCap) {
                  const uint32_t availableSpace = maxCap - current;
                  const uint32_t actualDelivered = (cargoTons > availableSpace) ? availableSpace : cargoTons;
                  targetPortStock->fetch_add(actualDelivered, std::memory_order_relaxed);
                }
                Logger::Log("[Feeder] Truck #%u deposited %d tons %s at Port Terminus (+%u company, +%u tariff, Stock: %u/%u)",
                            truck.id, truck.cargoWeightTons, cargoName, grossFee - portTariff, portTariff,
                            targetPortStock->load(std::memory_order_relaxed), maxCap);
                truck.cargoWeightTons = 0;
                truck.cargoType = 0;

                const auto &rNodes = g_customRoutes[truck.companyId].nodes;
                truck.state = TruckState::EN_ROUTE;
                truck.speed = 22.0f;
                truck.travelForward = false;
                truck.currentTargetNode =
                    (rNodes.size() > 1) ? rNodes.size() - 2 : 0;
                g_physicalFeedback[truck.id].travelForward.store(
                    false, std::memory_order_release);
                g_physicalFeedback[truck.id].targetWaypointIndex.store(
                    static_cast<uint32_t>(truck.currentTargetNode),
                    std::memory_order_release);
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              } else {
                handleSfUnload(truck);
                if (s_companyBalances[truck.companyId].load(
                        std::memory_order_relaxed) < 0) {
                  truck.stateTimer = 1600;
                  truck.speed = 0.0f;
                  g_physicalFeedback[truck.id].stopped.store(
                      true, std::memory_order_release);
                } else {
                  truck.state = TruckState::EN_ROUTE;
                  truck.speed = 22.0f;
                  g_physicalFeedback[truck.id].stopped.store(
                      false, std::memory_order_release);
                }
              }
            } else if (truck.state == TruckState::LOADING) {
              if (truck.id >= k_truckCount) {
                // Dispatch check: trucks should not target hubs that are >= 95% full
                std::atomic<uint32_t>* targetPortStock = nullptr;
                if (truck.companyId == 2) targetPortStock = &s_portFoodStock;
                else if (truck.companyId == 1) targetPortStock = &s_portFuelStock;
                else targetPortStock = &s_sfElectronicsStock;

                const uint32_t currentPortStock = targetPortStock->load(std::memory_order_relaxed);
                constexpr uint32_t portMaxCap = 500;
                if (currentPortStock >= (portMaxCap * 95) / 100) {
                  // Hub >= 95% full: hold truck at factory loading bay
                  truck.stateTimer = 400; // wait ~20s before re-checking
                  truck.speed = 0.0f;
                  g_physicalFeedback[truck.id].stopped.store(true, std::memory_order_release);
                  continue;
                }

                // Feeder Truck finished 6.0s loading at Node 0 Factory Hub
                if (truck.companyId == 2) {
                  truck.cargoType = 4; // Fresh Agricultural Produce / Grain
                  truck.cargoWeightTons = 25;
                  Logger::Log("[Feeder] Truck #%u finished loading 25 tons "
                              "Food at Agro Hub (Node 0)",
                              truck.id);
                } else if (truck.companyId == 1) {
                  truck.cargoType = 2; // Petrochemical / High-Octane Fuel
                  truck.cargoWeightTons = 25;
                  Logger::Log("[Feeder] Truck #%u finished loading 25 tons "
                              "Fuel at Refinery Hub (Node 0)",
                              truck.id);
                } else {
                  truck.cargoType = 3; // Intermodal Freight / Containers
                  truck.cargoWeightTons = 20;
                  Logger::Log("[Feeder] Truck #%u finished loading 20 tons "
                              "Freight at Freight Hub (Node 0)",
                              truck.id);
                }
                ApplyCargoComplianceAndLoading(truck);

                const auto &rNodes = g_customRoutes[truck.companyId].nodes;
                truck.state = TruckState::EN_ROUTE;
                truck.speed = 22.0f;
                truck.travelForward = true;
                truck.currentTargetNode = (rNodes.size() > 1) ? 1 : 0;
                g_physicalFeedback[truck.id].travelForward.store(
                    true, std::memory_order_release);
                g_physicalFeedback[truck.id].targetWaypointIndex.store(
                    static_cast<uint32_t>(truck.currentTargetNode),
                    std::memory_order_release);
                g_physicalFeedback[truck.id].stopped.store(
                    false, std::memory_order_release);
              } else {
                if (s_companyBalances[truck.companyId].load(
                        std::memory_order_relaxed) < 0) {
                  truck.stateTimer = 1600;
                  truck.speed = 0.0f;
                  g_physicalFeedback[truck.id].stopped.store(
                      true, std::memory_order_release);
                } else {
                  truck.state = TruckState::EN_ROUTE;
                  truck.speed = 22.0f;
                  truck.deadlineTicks =
                      33600; // 28 minutes deadline upon leaving Ocean Docks
                  truck.deadlinePenalized = false;
                  g_physicalFeedback[truck.id].stopped.store(
                      false, std::memory_order_release);
                }
              }
            } else if (truck.state == TruckState::INSPECTION) {
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            } else if (truck.state == TruckState::BROKEN_DOWN) {
              g_physicalFeedback[truck.id].brokenDown.store(
                  false, std::memory_order_release);
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            } else if (truck.state == TruckState::DESTROYED) {
              g_physicalFeedback[truck.id].destroyed.store(
                  false, std::memory_order_release);
              truck.currentTargetNode = 0;
              truck.travelForward = true;
              g_physicalFeedback[truck.id].targetWaypointIndex.store(
                  0, std::memory_order_release);
              g_physicalFeedback[truck.id].travelForward.store(
                  true, std::memory_order_release);
              const HighwayWaypoint &resetWp = s_highwayLoop[0];
              truck.x = resetWp.x;
              truck.y = resetWp.y;
              truck.z = resetWp.z;
              truck.fuel = 100.0f;
              truck.fatigue = 0.0f;
              if (truck.companyId == 2) {
                truck.cargoType = 4;
                truck.cargoWeightTons = 16 + (static_cast<int>(truck.id) % 7);
              } else if (truck.companyId == 1) {
                truck.cargoType = 2;
                truck.cargoWeightTons = 20 + (static_cast<int>(truck.id) % 7);
              } else {
                truck.cargoType = 1;
                truck.cargoWeightTons = 14 + (static_cast<int>(truck.id) % 5);
              }
              ApplyCargoComplianceAndLoading(truck);
              truck.state = TruckState::LOADING;
              truck.stateTimer = 1600; // ~80 seconds
              truck.speed = 0.0f;
            } else {
              truck.state = TruckState::EN_ROUTE;
              truck.speed = 22.0f;
              g_physicalFeedback[truck.id].stopped.store(
                  false, std::memory_order_release);
            }
          }
        } else {
          g_physicalFeedback[truck.id].stopped.store(false,
                                                     std::memory_order_release);
          const float fuelBurn =
              (truck.skillLevel >= 3) ? (0.004f * 0.7f) : 0.004f;
          const float fatigueGain =
              (truck.skillLevel >= 3) ? (0.0035f * 0.7f) : 0.0035f;
          truck.fuel = std::max(0.0f, truck.fuel - fuelBurn);
          truck.fatigue = std::min(100.0f, truck.fatigue + fatigueGain);

          if (truck.id < k_truckCount) {
            // Master Highway Interstate Hauler virtual traversal
            const HighwayWaypoint *targetWp =
                &s_highwayLoop[truck.currentTargetNode % s_waypointCount];
            float dx = targetWp->x - truck.x;
            float dy = targetWp->y - truck.y;
            float dz = targetWp->z - truck.z;
            float dist2D = std::sqrt(dx * dx + dy * dy);

            // When within 15 units of current target node
            if (dist2D < 15.0f) {
              if (truck.currentTargetNode == 0) {
                handleOceanDocksArrival(truck);
                truck.currentTargetNode =
                    (truck.currentTargetNode + 1) % s_waypointCount;
              } else if (truck.currentTargetNode == 85) {
                handleWeighStationInspection(truck);
                g_physicalFeedback[truck.id].stopped.store(
                    true, std::memory_order_release);
                truck.currentTargetNode =
                    (truck.currentTargetNode + 1) % s_waypointCount;
              } else if (truck.currentTargetNode == 135) {
                truck.state = TruckState::RESTING;
                truck.stateTimer = 500; // 25 seconds (500 ticks)
                truck.speed = 0.0f;
                truck.currentTargetNode =
                    (truck.currentTargetNode + 1) % s_waypointCount;
              } else if (truck.currentTargetNode == 240) {
                truck.state = TruckState::UNLOADING;
                truck.stateTimer = 1600; // ~80 seconds (1600 ticks)
                truck.speed = 0.0f;
                truck.currentTargetNode =
                    (truck.currentTargetNode + 1) % s_waypointCount;
              } else {
                truck.currentTargetNode =
                    (truck.currentTargetNode + 1) % s_waypointCount;
                targetWp =
                    &s_highwayLoop[truck.currentTargetNode % s_waypointCount];
                dx = targetWp->x - truck.x;
                dy = targetWp->y - truck.y;
                dz = targetWp->z - truck.z;
                dist2D = std::sqrt(dx * dx + dy * dy);
              }
              g_physicalFeedback[truck.id].targetWaypointIndex.store(
                  static_cast<uint32_t>(truck.currentTargetNode),
                  std::memory_order_release);
            }

            // Roadblock collision: check both current position and destination waypoint
            const bool blockedAtPosition = IsRoadblockBlockingWaypoint(truck.x, truck.y, 45.0f);
            const bool blockedAhead = (targetWp != nullptr) && IsRoadblockBlockingWaypoint(targetWp->x, targetWp->y, 35.0f);

            if (blockedAtPosition || blockedAhead) {
                truck.speed = 0.0f;
                truck.state = TruckState::RESTING;
                truck.stateTimer = 100; // Hold position
                continue; // Skip advancing along the route
            }

            if (truck.state == TruckState::EN_ROUTE && dist2D > 0.001f) {
              const float invDist = 1.0f / dist2D;
              const float moveStep = truck.speed * dt;
              truck.x += (dx * invDist) * moveStep;
              truck.y += (dy * invDist) * moveStep;
              truck.z += (dz * invDist) * moveStep;

              float hRad = std::atan2(-dx, dy);
              float hDeg = hRad * (180.0f / 3.14159265358979323846f);
              while (hDeg < 0.0f)
                hDeg += 360.0f;
              while (hDeg >= 360.0f)
                hDeg -= 360.0f;
              fleet[i].heading = hDeg;
            }
          } else {
            // Local Feeder Shuttle virtual traversal along company route
            const auto &rNodes = g_customRoutes[truck.companyId].nodes;
            if (rNodes.size() >= 2) {
              const auto &termNode = rNodes[rNodes.size() - 1];
              const float distTerm =
                  std::hypot(truck.x - termNode.x, truck.y - termNode.y);
              const float distPort =
                  std::hypot(truck.x - 2312.2f, truck.y - (-2252.0f));
              const auto &originNode = rNodes[0];
              const float distOrigin =
                  std::hypot(truck.x - originNode.x, truck.y - originNode.y);

              if ((distTerm < 15.0f || distPort < 15.0f) &&
                  truck.cargoWeightTons > 0 && truck.travelForward) {
                truck.state = TruckState::UNLOADING;
                truck.stateTimer = 120; // 6.0 seconds (120 ticks) hold position
                truck.speed = 0.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    true, std::memory_order_release);
              } else if (distOrigin < 15.0f && truck.cargoWeightTons == 0 &&
                         !truck.travelForward) {
                truck.state = TruckState::LOADING;
                truck.stateTimer = 120; // 6.0 seconds (120 ticks) hold position
                truck.speed = 0.0f;
                g_physicalFeedback[truck.id].stopped.store(
                    true, std::memory_order_release);
              } else {
                size_t curIdx = truck.currentTargetNode;
                if (curIdx >= rNodes.size())
                  curIdx = rNodes.size() - 1;
                const auto &targetNode = rNodes[curIdx];
                float dx = targetNode.x - truck.x;
                float dy = targetNode.y - truck.y;
                float dist2D = std::sqrt(dx * dx + dy * dy);

                if (dist2D < 15.0f) {
                  if (truck.travelForward) {
                    if (curIdx + 1 < rNodes.size()) {
                      truck.currentTargetNode = curIdx + 1;
                    } else {
                      if (truck.cargoWeightTons > 0) {
                        truck.state = TruckState::UNLOADING;
                        truck.stateTimer = 120; // 6.0 seconds
                        truck.speed = 0.0f;
                        g_physicalFeedback[truck.id].stopped.store(
                            true, std::memory_order_release);
                      }
                    }
                  } else {
                    if (curIdx > 0) {
                      truck.currentTargetNode = curIdx - 1;
                    } else {
                      if (truck.cargoWeightTons == 0) {
                        truck.state = TruckState::LOADING;
                        truck.stateTimer = 120; // 6.0 seconds
                        truck.speed = 0.0f;
                        g_physicalFeedback[truck.id].stopped.store(
                            true, std::memory_order_release);
                      }
                    }
                  }
                  g_physicalFeedback[truck.id].targetWaypointIndex.store(
                      static_cast<uint32_t>(truck.currentTargetNode),
                      std::memory_order_release);
                }

                if (truck.state == TruckState::EN_ROUTE && dist2D > 0.001f) {
                  const float invDist = 1.0f / dist2D;
                  const float moveStep = truck.speed * dt;
                  truck.x += (dx * invDist) * moveStep;
                  truck.y += (dy * invDist) * moveStep;
                  truck.z = targetNode.z;

                  float hRad = std::atan2(-dx, dy);
                  float hDeg = hRad * (180.0f / 3.14159265358979323846f);
                  while (hDeg < 0.0f)
                    hDeg += 360.0f;
                  while (hDeg >= 360.0f)
                    hDeg -= 360.0f;
                  truck.heading = hDeg;
                }
              }
            }
          }
        }
      }

      // Proximity checks for 3D materialization
      const float pDx = truck.x - playerX;
      const float pDy = truck.y - playerY;
      const float distToPlayer = std::sqrt(pDx * pDx + pDy * pDy);

      constexpr float STREAM_IN_DIST = 130.0f;
      constexpr float STREAM_OUT_DIST = 240.0f;

      // SPAWN TRIGGER: within 110 units and not yet materialized
      if (distToPlayer < STREAM_IN_DIST && !isPhysical &&
          !truck.spawnRequested && truck.state != TruckState::DESTROYED) {
        truck.spawnRequested = true;
        truck.spawnPendingTicks = 0;
        const AIDirective spawnDir{.type = AIDirectiveType::SpawnTruck,
                                   .truckId = truck.id,
                                   .targetX = truck.x,
                                   .targetY = truck.y,
                                   .targetZ = truck.z,
                                   .heading = truck.heading,
                                   .speed = truck.speed,
                                   .directiveId = ++directiveIdCounter,
                                   .targetWaypointIndex = static_cast<uint32_t>(
                                       truck.currentTargetNode)};
        g_aiDirectiveQueue.push(spawnDir);
      }
      // DESPAWN TRIGGER: beyond 140 units and currently materialized or spawn
      // was requested
      else if (distToPlayer > STREAM_OUT_DIST &&
               (isPhysical || truck.spawnRequested)) {
        truck.spawnRequested = false;
        truck.spawnPendingTicks = 0;
        if (isPhysical) {
          const AIDirective despawnDir{.type = AIDirectiveType::DespawnTruck,
                                       .truckId = truck.id,
                                       .targetX = 0.0f,
                                       .targetY = 0.0f,
                                       .targetZ = 0.0f,
                                       .heading = 0.0f,
                                       .speed = 0.0f,
                                       .directiveId = ++directiveIdCounter};
          g_aiDirectiveQueue.push(despawnDir);
        }
      } else if (truck.spawnRequested && !isPhysical) {
        truck.spawnPendingTicks++;
        if (truck.spawnPendingTicks > 60) {
          // Fallback timeout (~3s): reset spawnRequested so director can retry
          // cleanly
          truck.spawnRequested = false;
          truck.spawnPendingTicks = 0;
        }
      } else if (isPhysical) {
        truck.spawnPendingTicks = 0;
      }
    }

    // Publish tear-free snapshot for Web Worker /api/status
    const uint32_t currentSnap =
        g_truckSnapshotActiveIdx.load(std::memory_order_relaxed);
    const uint32_t writeSnap = 1 - currentSnap;
    for (size_t i = 0; i < k_totalTruckCount; ++i) {
      g_truckSnapshots[writeSnap][i] = {
          .id = fleet[i].id,
          .x = fleet[i].x,
          .y = fleet[i].y,
          .z = fleet[i].z,
          .heading = fleet[i].heading,
          .speed = fleet[i].speed,
          .isMaterialized = fleet[i].isMaterialized,
          .state = fleet[i].state,
          .fuel = fleet[i].fuel,
          .fatigue = fleet[i].fatigue,
          .cargoType = fleet[i].cargoType,
          .cargoWeightTons = fleet[i].cargoWeightTons,
          .deliveriesDone = fleet[i].deliveriesDone,
          .companyId = fleet[i].companyId,
          .driverName = fleet[i].driverName,
          .driverWallet = fleet[i].driverWallet,
          .deadlineTicks = fleet[i].deadlineTicks,
          .experience = fleet[i].experience,
          .skillLevel = fleet[i].skillLevel};
    }
    g_truckSnapshotActiveIdx.store(writeSnap, std::memory_order_release);

    // Process telemetry queue from game thread if any
    TelemetryState state{};
    while (g_aiTelemetryQueue.pop(state)) {
      g_totalTelemetryProcessed.fetch_add(1, std::memory_order_relaxed);
    }

    EconomyRetail::UpdateStoreRaids(CTimer::m_snTimeInMilliseconds, s_socialUnrest.load(std::memory_order_relaxed));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  EconomyTelemetry::StopTelemetryWorker();
}



// =============================================================================
//  MoonLoader JSON Bridge (Lock-Free IPC & Economy Processing)
// =============================================================================

void EnsureMunicipalConfigDir() {
  CreateDirectoryA("moonloader", nullptr);
  CreateDirectoryA("moonloader\\config", nullptr);
}


// =============================================================================
//  Custom Logistics Routes Persistence & Management
// =============================================================================

bool SaveCustomRoutesToFile() {
  std::string json;
  json.reserve(32768);
  json.append("{\n  \"routes\": [\n");
  char buf[512];
  for (size_t c = 0; c < 3; ++c) {
    if (c > 0)
      json.append(",\n");
    snprintf(buf, sizeof(buf),
             "    {\n      \"companyId\": %u,\n      \"color\": \"%s\",\n      "
             "\"nodes\": [\n",
             static_cast<unsigned int>(g_customRoutes[c].companyId),
             g_customRoutes[c].color);
    json.append(buf);
    for (size_t n = 0; n < g_customRoutes[c].nodes.size(); ++n) {
      const auto &node = g_customRoutes[c].nodes[n];
      snprintf(buf, sizeof(buf),
               "        { \"x\": %.1f, \"y\": %.1f, \"z\": %.1f }%s\n", node.x,
               node.y, node.z,
               (n + 1 < g_customRoutes[c].nodes.size()) ? "," : "");
      json.append(buf);
    }
    json.append("      ]\n    }");
  }
  json.append("\n  ]\n}\n");

  try {
    std::ofstream file("logistics_routes.json",
                       std::ios::out | std::ios::trunc);
    if (file.is_open()) {
      file.write(json.data(), json.size());
      file.flush();
      file.close();
      Logger::Log("[RoutePersistence] Saved custom routes to "
                  "logistics_routes.json via std::ofstream (%zu bytes)",
                  json.size());

      // Mirror copy to moonloader/config if accessible
      std::ofstream mlFile("moonloader\\config\\logistics_routes.json",
                           std::ios::out | std::ios::trunc);
      if (mlFile.is_open()) {
        mlFile.write(json.data(), json.size());
        mlFile.flush();
        mlFile.close();
      }
      return true;
    } else {
      Logger::Log("[RoutePersistence] ERROR: Failed to open "
                  "logistics_routes.json for writing");
    }
  } catch (const std::exception &ex) {
    Logger::Log(
        "[RoutePersistence] Exception writing logistics_routes.json: %s",
        ex.what());
  }
  return false;
}

bool LoadCustomRoutesJson() {
  std::string content;
  std::ifstream file("logistics_routes.json");
  if (!file.is_open()) {
    file.open("moonloader\\config\\logistics_routes.json");
  }
  if (file.is_open()) {
    content.assign((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());
    file.close();
  } else {
    return false;
  }
  if (content.empty())
    return false;

  std::lock_guard<std::recursive_mutex> lock(g_customRoutesMutex);
  for (size_t c = 0; c < 3; ++c) {
    g_customRoutes[c].companyId = static_cast<uint8_t>(c);
    g_customRoutes[c].nodes.clear();
    g_customRoutes[c].active = false;
  }

  size_t routesPos = content.find("\"routes\":");
  if (routesPos == std::string::npos)
    return false;

  size_t cur = routesPos;
  for (size_t i = 0; i < 3; ++i) {
    size_t cPos = content.find("\"companyId\":", cur);
    if (cPos == std::string::npos)
      cPos = content.find("\"company_id\":", cur);
    if (cPos == std::string::npos)
      break;

    int cId = std::atoi(content.c_str() + cPos + 12);
    if (cId < 0 || cId >= 3) {
      cur = cPos + 13;
      continue;
    }

    // Color
    size_t colPos = content.find("\"color\":", cPos);
    if (colPos != std::string::npos && colPos < cPos + 80) {
      size_t q1 = content.find('"', colPos + 8);
      if (q1 != std::string::npos) {
        size_t q2 = content.find('"', q1 + 1);
        if (q2 != std::string::npos) {
          std::string col = content.substr(q1 + 1, q2 - q1 - 1);
          strncpy_s(g_customRoutes[cId].color, col.c_str(),
                    sizeof(g_customRoutes[cId].color) - 1);
        }
      }
    }

    // Nodes
    size_t nPos = content.find("\"nodes\":", cPos);
    if (nPos != std::string::npos) {
      size_t openBracket = content.find('[', nPos);
      size_t closeBracket = content.find(
          ']', openBracket != std::string::npos ? openBracket : nPos);
      if (openBracket != std::string::npos &&
          closeBracket != std::string::npos) {
        size_t nCur = openBracket + 1;
        while (nCur < closeBracket) {
          size_t objStart = content.find('{', nCur);
          if (objStart == std::string::npos || objStart >= closeBracket)
            break;
          size_t objEnd = content.find('}', objStart);
          if (objEnd == std::string::npos || objEnd > closeBracket)
            break;

          std::string objStr = content.substr(objStart, objEnd - objStart + 1);
          CustomRouteNode node{};
          node.z = 15.0f;

          size_t xPos = objStr.find("\"x\":");
          if (xPos != std::string::npos) {
            node.x = static_cast<float>(
                std::strtof(objStr.c_str() + xPos + 4, nullptr));
          }
          size_t yPos = objStr.find("\"y\":");
          if (yPos != std::string::npos) {
            node.y = static_cast<float>(
                std::strtof(objStr.c_str() + yPos + 4, nullptr));
          }
          size_t zPos = objStr.find("\"z\":");
          if (zPos != std::string::npos) {
            node.z = static_cast<float>(
                std::strtof(objStr.c_str() + zPos + 4, nullptr));
          }

          g_customRoutes[cId].nodes.push_back(node);
          nCur = objEnd + 1;
        }
        cur = closeBracket + 1;
      } else {
        cur = nPos + 8;
      }
    } else {
      cur = cPos + 13;
    }

    g_customRoutes[cId].active = (g_customRoutes[cId].nodes.size() >= 2);
  }
  Logger::Log("[RoutePersistence] Loaded custom routes from disk into memory");
  return true;
}

void InitCustomRoutes() {
  std::lock_guard<std::recursive_mutex> lock(g_customRoutesMutex);
  for (size_t c = 0; c < 3; ++c) {
    g_customRoutes[c].companyId = static_cast<uint8_t>(c);
    g_customRoutes[c].nodes.clear();
    g_customRoutes[c].active = false;
  }
  strncpy_s(g_customRoutes[0].color, "#38bdf8",
            sizeof(g_customRoutes[0].color) - 1);
  strncpy_s(g_customRoutes[1].color, "#f59e0b",
            sizeof(g_customRoutes[1].color) - 1);
  strncpy_s(g_customRoutes[2].color, "#22c55e",
            sizeof(g_customRoutes[2].color) - 1);

  // If logistics_routes.json exists on disk, parse and load it into memory
  LoadCustomRoutesJson();
}

bool SaveSingleCustomRoute(uint8_t companyId, const char *color,
                           const std::vector<CustomRouteNode> &nodes) {
  if (companyId >= 3)
    return false;
  {
    std::lock_guard<std::recursive_mutex> lock(g_customRoutesMutex);
    g_customRoutes[companyId].companyId = companyId;
    if (color && *color) {
      strncpy_s(g_customRoutes[companyId].color, color,
                sizeof(g_customRoutes[companyId].color) - 1);
    }
    g_customRoutes[companyId].nodes = nodes;
    g_customRoutes[companyId].active = (nodes.size() >= 2);

    // Immediately reset path progress for assigned feeder trucks (IDs 24..29)
    // Node 0 dynamically acts as that company's active production facility
    // (loading zone)
    if (nodes.size() >= 2) {
      for (size_t i = k_truckCount; i < k_totalTruckCount; ++i) {
        if (s_fleet[i].companyId == companyId) {
          s_fleet[i].x = nodes[0].x;
          s_fleet[i].y = nodes[0].y;
          s_fleet[i].z = nodes[0].z;
          s_fleet[i].travelForward = true;
          s_fleet[i].currentTargetNode = 1;
          s_fleet[i].state = TruckState::LOADING;
          s_fleet[i].stateTimer = 120; // 6.0s hold position / loading at Node 0
          s_fleet[i].speed = 0.0f;
          s_fleet[i].cargoWeightTons = 0;
          s_fleet[i].cargoType = 0;
          g_physicalFeedback[i].targetWaypointIndex.store(
              0, std::memory_order_release);
          g_physicalFeedback[i].travelForward.store(true,
                                                    std::memory_order_release);
          g_physicalFeedback[i].stopped.store(true, std::memory_order_release);
        }
      }
    }
  }

  return SaveCustomRoutesToFile();
}

std::string ExportCustomRoutesJson() {
  std::lock_guard<std::recursive_mutex> lock(g_customRoutesMutex);
  bool hasAnyNodes = false;
  for (size_t c = 0; c < 3; ++c) {
    if (!g_customRoutes[c].nodes.empty()) {
      hasAnyNodes = true;
      break;
    }
  }
  if (!hasAnyNodes) {
    return "{\"status\":\"ok\",\"routes\":[]}";
  }

  std::string json = "{\"status\":\"ok\",\"routes\":[";
  char buf[512];
  bool first = true;
  for (size_t c = 0; c < 3; ++c) {
    if (g_customRoutes[c].nodes.empty())
      continue;
    if (!first)
      json.push_back(',');
    first = false;
    snprintf(buf, sizeof(buf),
             "{\"companyId\":%u,\"color\":\"%s\",\"active\":%s,\"nodes\":[",
             static_cast<unsigned int>(g_customRoutes[c].companyId),
             g_customRoutes[c].color,
             g_customRoutes[c].active ? "true" : "false");
    json.append(buf);
    for (size_t n = 0; n < g_customRoutes[c].nodes.size(); ++n) {
      if (n > 0)
        json.push_back(',');
      const auto &node = g_customRoutes[c].nodes[n];
      snprintf(buf, sizeof(buf), "{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f}", node.x,
               node.y, node.z);
      json.append(buf);
    }
    json.append("]}");
  }
  json.append("]}");
  return json;
}

bool ParseRouteJsonPayload(const char *body, uint8_t &outCompanyId,
                           std::string &outColor,
                           std::vector<CustomRouteNode> &outNodes) {
  if (!body)
    return false;
  const std::string s(body);

  int cId = 0;
  size_t cidPos = s.find("\"companyId\":");
  if (cidPos == std::string::npos)
    cidPos = s.find("\"company_id\":");
  if (cidPos != std::string::npos) {
    cId = std::atoi(s.c_str() + cidPos + 12);
  }
  if (cId < 0 || cId >= 3)
    cId = 0;
  outCompanyId = static_cast<uint8_t>(cId);

  outColor = (outCompanyId == 0) ? "#38bdf8"
                                 : (outCompanyId == 1 ? "#f59e0b" : "#22c55e");
  size_t colPos = s.find("\"color\":");
  if (colPos != std::string::npos) {
    size_t q1 = s.find('"', colPos + 8);
    if (q1 != std::string::npos) {
      size_t q2 = s.find('"', q1 + 1);
      if (q2 != std::string::npos) {
        outColor = s.substr(q1 + 1, q2 - q1 - 1);
      }
    }
  }

  outNodes.clear();
  size_t nodesPos = s.find("\"nodes\":");
  if (nodesPos != std::string::npos) {
    size_t openBracket = s.find('[', nodesPos);
    size_t closeBracket =
        s.find(']', openBracket != std::string::npos ? openBracket : nodesPos);
    if (openBracket != std::string::npos && closeBracket != std::string::npos) {
      size_t cur = openBracket + 1;
      while (cur < closeBracket) {
        size_t objStart = s.find('{', cur);
        if (objStart == std::string::npos || objStart >= closeBracket)
          break;
        size_t objEnd = s.find('}', objStart);
        if (objEnd == std::string::npos || objEnd > closeBracket)
          break;

        std::string objStr = s.substr(objStart, objEnd - objStart + 1);
        CustomRouteNode node{};
        node.z = 12.0f;

        size_t xPos = objStr.find("\"x\":");
        if (xPos != std::string::npos) {
          node.x = static_cast<float>(
              std::strtof(objStr.c_str() + xPos + 4, nullptr));
        }
        size_t yPos = objStr.find("\"y\":");
        if (yPos != std::string::npos) {
          node.y = static_cast<float>(
              std::strtof(objStr.c_str() + yPos + 4, nullptr));
        }
        size_t zPos = objStr.find("\"z\":");
        if (zPos != std::string::npos) {
          node.z = static_cast<float>(
              std::strtof(objStr.c_str() + zPos + 4, nullptr));
        }

        outNodes.push_back(node);
        cur = objEnd + 1;
      }
    }
  }

  return true;
}

void ExportMoonLoaderJsonState() {
  CreateDirectoryA("moonloader\\config", nullptr);

  int raidedStoreId = -1;
  const char *raidedStoreName = "None";
  float rx = 0.0f, ry = 0.0f, rz = 0.0f;
  uint32_t rStock = 0;
  bool rUnderRaid = false;
  for (size_t s = 0; s < 20; ++s) {
    if (g_retailStores[s].isUnderRaid.load(std::memory_order_relaxed)) {
      raidedStoreId = static_cast<int>(s);
      raidedStoreName = g_retailStores[s].name;
      rx = g_retailStores[s].posX;
      ry = g_retailStores[s].posY;
      rz = k_retailStoreGroundZ[s];
      rStock = g_retailStores[s].localStock.load(std::memory_order_relaxed);
      rUnderRaid = true;
      break;
    }
  }

  const float fuelMult = s_fuelPriceMultiplier.load(std::memory_order_relaxed);
  const float storeMult =
      s_storePriceMultiplier.load(std::memory_order_relaxed);
  const int64_t bal0 = s_companyBalances[0].load(std::memory_order_relaxed);
  const int64_t bal1 = s_companyBalances[1].load(std::memory_order_relaxed);
  const int64_t bal2 = s_companyBalances[2].load(std::memory_order_relaxed);
  const uint32_t stockTimber = s_sfTimberStock.load(std::memory_order_relaxed);
  const uint32_t stockFuel = s_sfFuelStock.load(std::memory_order_relaxed);
  const uint32_t stockElec =
      s_sfElectronicsStock.load(std::memory_order_relaxed);
  const uint32_t stockFood = s_sfFoodStock.load(std::memory_order_relaxed);
  const int8_t owner0 = s_assetOwners[0].load(std::memory_order_relaxed);
  const int8_t owner1 = s_assetOwners[1].load(std::memory_order_relaxed);
  const int8_t owner2 = s_assetOwners[2].load(std::memory_order_relaxed);

  CPed *player = FindPlayerPed();
  const int playerMoney =
      (player && player->m_pPlayerData) ? player->m_pPlayerData->m_nMoney : 0;
  const CVector pPos =
      player ? (player->m_pVehicle ? player->m_pVehicle->GetPosition()
                                   : player->GetPosition())
             : CVector(0.0f, 0.0f, 0.0f);
  const uint32_t ms = CTimer::m_snTimeInMilliseconds;

  std::string json;
  json.reserve(16384);
  char buf[2048];

  snprintf(buf, sizeof(buf),
           "{\n"
           "  \"timestamp\": %u,\n"
           "  \"raidedStoreId\": %d,\n"
           "  \"raidedStoreName\": \"%s\",\n"
           "  \"raidedStoreCoords\": { \"x\": %.1f, \"y\": %.1f, \"z\": %.1f },\n"
           "  \"localStock\": %u,\n"
           "  \"isUnderRaid\": %s,\n"
           "  \"multipliers\": { \"store\": %.2f, \"fuel\": %.2f },\n"
           "  \"commodities\": { \"timber\": %u, \"fuel\": %u, "
           "\"electronics\": %u, \"food\": %u },\n"
           "  \"companies\": [\n"
           "    { \"id\": 0, \"name\": \"%s\", \"balance\": %lld, \"color\": "
           "\"%s\" },\n"
           "    { \"id\": 1, \"name\": \"%s\", \"balance\": %lld, \"color\": "
           "\"%s\" },\n"
           "    { \"id\": 2, \"name\": \"%s\", \"balance\": %lld, \"color\": "
           "\"%s\" }\n"
           "  ],\n"
           "  \"assets\": [\n"
           "    { \"id\": 0, \"name\": \"%s\", \"cost\": %u, \"owner\": %d },\n"
           "    { \"id\": 1, \"name\": \"%s\", \"cost\": %u, \"owner\": %d },\n"
           "    { \"id\": 2, \"name\": \"%s\", \"cost\": %u, \"owner\": %d }\n"
           "  ],\n"
           "  \"player\": {\n"
           "    \"x\": %.2f, \"y\": %.2f, \"z\": %.2f,\n"
           "    \"cash\": %d\n"
           "  },\n",
           ms, raidedStoreId, raidedStoreName, rx, ry, rz, rStock,
           rUnderRaid ? "true" : "false",
           storeMult, fuelMult, stockTimber, stockFuel, stockElec,
           stockFood, k_companies[0].name, static_cast<long long>(bal0),
           k_companies[0].color, k_companies[1].name,
           static_cast<long long>(bal1), k_companies[1].color,
           k_companies[2].name, static_cast<long long>(bal2),
           k_companies[2].color, k_assets[0].name, k_assets[0].cost, owner0,
           k_assets[1].name, k_assets[1].cost, owner1, k_assets[2].name,
           k_assets[2].cost, owner2, pPos.x, pPos.y, pPos.z, playerMoney);
  json.append(buf);

  // 1. Legacy gasStations array (all Cat 1 Fuel stores)
  json.append("  \"gasStations\": [\n");
  bool firstGas = true;
  for (size_t i = 0; i < 20; ++i) {
    auto &store = g_retailStores[i];
    if (store.categoryId == 1) {
      float groundZ = k_retailStoreGroundZ[i];
      if (!firstGas)
        json.append(",\n");
      firstGas = false;

      const int finalPrice = CalculateRetailStorePrice(store.id);

      snprintf(buf, sizeof(buf),
               "    { \"id\": %u, \"shop_index\": %u, \"name\": \"%s\", \"x\": "
               "%.2f, \"y\": %.2f, \"z\": %.2f, \"refuelCost\": %d, "
               "\"patchCost\": %d, \"kitCost\": %d }",
               store.id, store.id, store.name, store.posX, store.posY, groundZ,
               finalPrice, static_cast<int>(300 * fuelMult),
               static_cast<int>(150 * fuelMult));
      json.append(buf);
    }
  }
  json.append("\n  ],\n");

  // 2. Legacy commercialStores array (Cat 0, 2, 3 stores)
  json.append("  \"commercialStores\": [\n");
  bool firstComm = true;
  for (size_t i = 0; i < 20; ++i) {
    auto &store = g_retailStores[i];
    if (store.categoryId != 1) {
      float groundZ = k_retailStoreGroundZ[i];
      if (!firstComm)
        json.append(",\n");
      firstComm = false;
      snprintf(
          buf, sizeof(buf),
          "    { \"id\": %u, \"shop_index\": %u, \"name\": \"%s\", \"x\": "
          "%.2f, \"y\": %.2f, \"z\": %.2f, \"armorCost\": %d, \"medkitCost\": "
          "%d, \"smgCost\": %d, \"deagleCost\": %d, \"shotgunCost\": %d, "
          "\"m4Cost\": %d }",
          store.id, store.id, store.name, store.posX, store.posY, groundZ,
          static_cast<int>(200 * storeMult), static_cast<int>(100 * storeMult),
          static_cast<int>(350 * storeMult), static_cast<int>(500 * storeMult),
          static_cast<int>(650 * storeMult), static_cast<int>(900 * storeMult));
      json.append(buf);
    }
  }
  json.append("\n  ],\n");

  // 3. Master stores array (all 20 stores: id, name, cat, dist, x, y, z, stock,
  // cap, isBankrupt, finalPrice)
  json.append("  \"stores\": [\n");
  for (size_t i = 0; i < 20; ++i) {
    auto &store = g_retailStores[i];
    float groundZ = k_retailStoreGroundZ[i];

    const uint32_t stock = store.localStock.load(std::memory_order_relaxed);
    const uint32_t cap = store.maxCapacity.load(std::memory_order_relaxed);
    const bool isBankrupt = store.isBankrupt.load(std::memory_order_relaxed);

    const int finalPrice = CalculateRetailStorePrice(store.id);

    snprintf(buf, sizeof(buf),
             "    { \"id\": %u, \"name\": \"%s\", \"cat\": %u, \"dist\": %u, "
             "\"x\": %.2f, \"y\": %.2f, \"z\": %.2f, \"stock\": %u, \"cap\": "
             "%u, \"isBankrupt\": %s, \"isUnderRaid\": %s, \"isRansacked\": %s, \"finalPrice\": %d }%s\n",
             store.id, store.name, store.categoryId, store.districtId,
             store.posX, store.posY, groundZ, stock, cap,
             isBankrupt ? "true" : "false",
             store.isUnderRaid.load(std::memory_order_relaxed) ? "true" : "false",
             store.isRansacked.load(std::memory_order_relaxed) ? "true" : "false",
             finalPrice,
             (i + 1 < 20) ? "," : "");
    json.append(buf);
  }
  json.append("  ]\n}\n");

  FILE *fp = nullptr;
  if (fopen_s(&fp, "moonloader\\config\\economy_state.tmp", "w") == 0 && fp) {
    fwrite(json.data(), 1, json.size(), fp);
    fclose(fp);
    MoveFileExA("moonloader\\config\\economy_state.tmp",
                "moonloader\\config\\economy_state.json",
                MOVEFILE_REPLACE_EXISTING);
  }
}

void ProcessMoonLoaderRequests(CPed *player) {
  if (!player || !player->m_pPlayerData || player->m_fHealth <= 0.0f)
    return;

  FILE *fp = nullptr;
  if (fopen_s(&fp, "moonloader\\config\\economy_requests.json", "r") != 0 ||
      !fp) {
    return;
  }

  char reqBuf[2048] = {0};
  size_t bytesRead = fread(reqBuf, 1, sizeof(reqBuf) - 1, fp);
  fclose(fp);
  reqBuf[bytesRead] = '\0';

  // Delete request file immediately so it's only processed once
  DeleteFileA("moonloader\\config\\economy_requests.json");

  if (bytesRead == 0)
    return;

  const std::string content(reqBuf);

  // 1. Parse store_id (0..19)
  int storeId = -1;
  size_t sPos = content.find("\"store_id\":");
  if (sPos != std::string::npos) {
    storeId = std::atoi(content.c_str() + sPos + 11);
  } else {
    sPos = content.find("\"shop_index\":");
    if (sPos != std::string::npos) {
      storeId = std::atoi(content.c_str() + sPos + 13);
    }
  }

  if (storeId < 0 || storeId >= 20) {
    return;
  }

  auto &store = g_retailStores[storeId];
  const uint32_t currentStock =
      store.localStock.load(std::memory_order_relaxed);
  const uint32_t maxCap = store.maxCapacity.load(std::memory_order_relaxed);
  const bool isBankrupt = store.isBankrupt.load(std::memory_order_relaxed);

  // 2. Parse cost
  int cost = 0;
  size_t cPos = content.find("\"cost\":");
  if (cPos != std::string::npos) {
    cost = std::atoi(content.c_str() + cPos + 7);
  }
  if (cost <= 0) {
    cost = CalculateRetailStorePrice(store.id);
  }

  // 3. Parse item_type
  std::string itemType;
  size_t itPos = content.find("\"item_type\":");
  if (itPos != std::string::npos) {
    size_t q1 = content.find('"', itPos + 12);
    if (q1 != std::string::npos) {
      size_t q2 = content.find('"', q1 + 1);
      if (q2 != std::string::npos) {
        itemType = content.substr(q1 + 1, q2 - q1 - 1);
      }
    }
  } else {
    size_t aPos = content.find("\"action\":");
    if (aPos != std::string::npos) {
      size_t q1 = content.find('"', aPos + 9);
      if (q1 != std::string::npos) {
        size_t q2 = content.find('"', q1 + 1);
        if (q2 != std::string::npos) {
          itemType = content.substr(q1 + 1, q2 - q1 - 1);
        }
      }
    }
  }

  if (itemType.empty()) {
    switch (store.categoryId) {
    case 0:
      itemType = "heal";
      break;
    case 1:
      itemType = "refuel";
      break;
    case 2:
      itemType = "armor";
      break;
    case 3:
      itemType = "ammo";
      break;
    default:
      itemType = "heal";
      break;
    }
  }

  // 4. Validation
  if (isBankrupt || currentStock == 0 || store.isRansacked.load(std::memory_order_relaxed)) {
    CHud::SetHelpMessage("~r~Store Ransacked!~w~", true, false, false);
    return;
  }

  if (player->m_pPlayerData->m_nMoney < cost) {
    return;
  }

  // 5. Deduct cash and stock
  player->m_pPlayerData->m_nMoney -= cost;
  uint32_t unitsToDeduct = 1;
  if (store.categoryId == 1 || store.categoryId == 3) {
    unitsToDeduct = (currentStock >= 2) ? 2 : currentStock;
  }
  store.localStock.fetch_sub(unitsToDeduct, std::memory_order_relaxed);

  // 6. Execute in-game reward safely without mission audio opcodes
  if (store.categoryId == 1 || itemType == "refuel" ||
      itemType == "armor_patch" || itemType == "jerry_can") {
    if (player->m_pVehicle) {
      player->m_pVehicle->m_fHealth = 1000.0f;
      player->m_pVehicle->ExtinguishCarFire();
      Command<Commands::FIX_CAR>(player->m_pVehicle);
    } else {
      player->m_fArmour = 50.0f;
    }
  } else if (itemType == "heal" || itemType == "medkit" ||
             itemType == "rations" || store.categoryId == 0 ||
             store.categoryId == 2) {
    player->m_fHealth = player->m_fMaxHealth;
    if (store.categoryId == 2 || itemType == "rations") {
      player->m_fArmour = (std::max)(player->m_fArmour, 50.0f);
    }
  } else if (itemType == "armor") {
    if (player->m_pVehicle) {
      player->m_pVehicle->m_fHealth = 1500.0f;
    } else {
      player->m_fArmour = 100.0f;
    }
  } else if (itemType == "ammo" || itemType == "micro_uzi" ||
             itemType == "deagle" || itemType == "shotgun" ||
             itemType == "m4" || store.categoryId == 3) {
    player->m_fArmour = 100.0f;
    player->GiveWeapon(WEAPONTYPE_MICRO_UZI, 150, true);
  }

  // 7. Record financial metrics
  RecordRetailStoreDirectPurchase(store, cost, unitsToDeduct);

  Logger::Log("[MoonBridge] Executed store %u purchase: %s for $%d", storeId,
              itemType.c_str(), cost);
}



// =============================================================================
//  Physical Trucks Lifecycle & Directives Management
// =============================================================================

static uint32_t s_truckPhysicalHandles[k_totalTruckCount] = {0};
static uint32_t s_truckTrailerHandles[k_totalTruckCount] = {0};
static AIDirective s_pendingSpawnDirective{};
static bool s_hasPendingSpawn = false;
static uint32_t s_pendingSpawnRetries = 0;

bool IsAreaClearOfVehicles(const CVector &pos, float radius) {
  if (!CPools::ms_pVehiclePool)
    return true;

  // Verify player position is not within 15.0f of pos to prevent spawning on CJ
  CPed *player = FindPlayerPed();
  if (player) {
    const CVector pPos = player->m_pVehicle ? player->m_pVehicle->GetPosition()
                                            : player->GetPosition();
    const float pDx = pPos.x - pos.x;
    const float pDy = pPos.y - pos.y;
    if ((pDx * pDx + pDy * pDy) < (15.0f * 15.0f)) {
      return false;
    }
  }

  const float radiusSq = radius * radius;
  const int poolSize = CPools::ms_pVehiclePool->m_nSize;
  for (int i = 0; i < poolSize; ++i) {
    CVehicle *veh = CPools::ms_pVehiclePool->GetAt(i);
    if (veh) {
      const CVector &vPos = veh->GetPosition();
      const float dx = vPos.x - pos.x;
      const float dy = vPos.y - pos.y;
      if ((dx * dx + dy * dy) < radiusSq) {
        return false;
      }
    }
  }

  return true;
}

void ProcessTruckDirectives() {
  // Persistent retry state for model streaming in the Game Thread
  constexpr size_t k_aiDrainBudget = 1;
  size_t aiDrained = 0;

  while (aiDrained < k_aiDrainBudget) {
    AIDirective directive{};
    bool hasDirective = false;

    if (s_hasPendingSpawn) {
      // Check if player moved out of streaming range or max retries reached
      const float pX = g_playerX.load(std::memory_order_relaxed);
      const float pY = g_playerY.load(std::memory_order_relaxed);
      const float dX = s_pendingSpawnDirective.targetX - pX;
      const float dY = s_pendingSpawnDirective.targetY - pY;
      if ((dX * dX + dY * dY) > (150.0f * 150.0f) ||
          s_pendingSpawnRetries >= 60) {
        s_hasPendingSpawn = false;
        s_pendingSpawnRetries = 0;
      } else {
        directive = s_pendingSpawnDirective;
        hasDirective = true;
        s_pendingSpawnRetries++;
      }
    }

    if (!hasDirective) {
      if (!g_aiDirectiveQueue.pop(directive)) {
        break;
      }
      g_totalDirectivesExecuted.fetch_add(1, std::memory_order_relaxed);
      hasDirective = true;
    }

    aiDrained++;

    switch (directive.type) {
    case AIDirectiveType::SpawnTruck: {
      const uint32_t id = directive.truckId;
      if (id < k_totalTruckCount && CPools::ms_pVehiclePool) {
        // Skip if already materialized and valid in pool
        if (s_truckPhysicalHandles[id] != 0 &&
            CPools::ms_pVehiclePool->GetAtRef(s_truckPhysicalHandles[id])) {
          s_hasPendingSpawn = false;
          s_pendingSpawnRetries = 0;
          break;
        }

        int modelId = MODEL_LINERUNNER;
        constexpr int driverModelId = MODEL_WMYMECH;
        int trailerModelId = MODEL_ARTICT1;

        if (id < k_truckCount) {
          // Tier 1: Master Interstate Haulers (s_highwayLoop)
          modelId = k_truckModels[id % 3];
          if (modelId == MODEL_TANKER) {
            trailerModelId = MODEL_PETROTR;
          } else {
            trailerModelId = (id % 2 == 0) ? MODEL_ARTICT1 : MODEL_ARTICT2;
          }
        } else {
          // Tier 2: Dedicated 3D Feeder Fleet for ALL 3 Companies
          const uint8_t compId =
              g_physicalFeedback[id].companyId.load(std::memory_order_relaxed) %
              3;
          if (compId == 0) {
            // Company 0 (Ocean Docks Logistics):
            // Vehicle: Linerunner / Roadtrain (ID 515) + Container/Freight
            // Trailer (ID 435/450)
            if (id % 2 == 0) {
              modelId = MODEL_ROADTRAIN;
              trailerModelId = MODEL_ARTICT1; // Container trailer 435
            } else {
              modelId = MODEL_LINERUNNER;
              trailerModelId = MODEL_ARTICT2; // Freight trailer 450
            }
          } else if (compId == 1) {
            // Company 1 (Bone County Petroleum):
            // Vehicle: Tanker (ID 514) + Petrochem Tanker Trailer (ID 584)
            modelId = MODEL_TANKER;
            trailerModelId = MODEL_PETROTR;
          } else {
            // Company 2 (Agro Food):
            // Vehicle: Flatbed (ID 455) or Linerunner (ID 515) + Farm Goods
            // Trailer
            if (id % 2 == 0) {
              modelId = MODEL_FLATBED; // Flatbed rigid hauler (ID 455)
              trailerModelId = 0;      // Rigid vehicle, no separate trailer
            } else {
              modelId = MODEL_LINERUNNER;
              trailerModelId = MODEL_FARMTR1; // Farm Goods Trailer (ID 610)
            }
          }
        }

        // Explicitly request models in advance with priority
        CStreaming::RequestModel(modelId,
                                 GAME_REQUIRED | STREAMING_PRIORITY_REQUEST);
        CStreaming::RequestModel(driverModelId,
                                 GAME_REQUIRED | STREAMING_PRIORITY_REQUEST);
        if (trailerModelId > 0) {
          CStreaming::RequestModel(trailerModelId,
                                   GAME_REQUIRED | STREAMING_PRIORITY_REQUEST);
        }

        const bool tractorLoaded =
            (CStreaming::ms_aInfoForModel[modelId].m_nLoadState ==
             LOADSTATE_LOADED);
        const bool driverLoaded =
            (CStreaming::ms_aInfoForModel[driverModelId].m_nLoadState ==
             LOADSTATE_LOADED);
        const bool trailerLoaded =
            (trailerModelId <= 0 ||
             CStreaming::ms_aInfoForModel[trailerModelId].m_nLoadState ==
                 LOADSTATE_LOADED);

        if (!tractorLoaded || !driverLoaded || !trailerLoaded) {
          // Retry directive on subsequent frame instead of discarding it
          s_pendingSpawnDirective = directive;
          s_hasPendingSpawn = true;
          break;
        }

        // Fully loaded: clear pending retry state
        s_hasPendingSpawn = false;
        s_pendingSpawnRetries = 0;

        const float hRad =
            directive.heading * (3.14159265358979323846f / 180.0f);
        const float fwdX = std::sin(-hRad);
        const float fwdY = std::cos(-hRad);

        CVector spawnPos(directive.targetX, directive.targetY,
                         directive.targetZ);
        bool foundSlot = false;

        // Scan base coordinates and forward slots (+15.0f up to +45.0f) for
        // clearance
        for (int step = 0; step <= 3; ++step) {
          const float stepOffset = step * 15.0f;
          CVector candidatePos(directive.targetX + fwdX * stepOffset,
                               directive.targetY + fwdY * stepOffset,
                               directive.targetZ);
          if (IsAreaClearOfVehicles(candidatePos, 18.0f)) {
            spawnPos = candidatePos;
            foundSlot = true;
            break;
          }
        }

        if (!foundSlot) {
          // All forward slots occupied by dense traffic; abort spawn for this
          // frame
          g_physicalFeedback[id].active.store(false, std::memory_order_release);
          g_physicalFeedback[id].isSpawned.store(false, std::memory_order_release);
          s_pendingSpawnDirective = directive;
          s_hasPendingSpawn = true;
          break;
        }

        // Account for 3D Ground Elevation (Z-coordinate)
        const float groundZ =
            CWorld::FindGroundZForCoord(spawnPos.x, spawnPos.y);
        if (groundZ > -100.0f) {
          spawnPos.z = groundZ;
        }

        CAutomobile *veh = new CAutomobile(modelId, 2, true);
        if (veh) {
          veh->m_nCreatedBy = 2;
          veh->m_eDoorLock = DOORLOCK_UNLOCKED;
          veh->m_fHealth = 1000.0f;
          veh->bEngineOn = true;
          veh->m_nHandbrakeOn = false;

          spawnPos.z = groundZ + 0.2f;
          veh->SetOrientation(0.0f, 0.0f, hRad);
          veh->Teleport(spawnPos);
          veh->m_nStatus = eEntityStatus::STATUS_PHYSICS;

          // Corporate Fleet Colors & Visual Styling:
          // Company 0 (Ocean Docks Haulage): Primary 2 (Classic Blue),
          // Secondary 1 (White). Company 1 (Bone County Fuel): Primary 6 (Deep
          // Yellow/Orange), Secondary 0 (Black). Company 2 (San Fierro
          // Freight): Primary 147 (Deep Purple), Secondary 1 (White).
          veh->m_nPrimaryColor = (id % 3 == 0) ? 2 : ((id % 3 == 1) ? 6 : 147);
          veh->m_nSecondaryColor = (id % 3 == 1) ? 0 : 1;

          CWorld::Add(veh);
          veh->PlaceOnRoadProperly();
          veh->UpdateRwMatrix();

          CPed *driver = new CPed(PED_TYPE_CIVMALE);
          driver->SetModelIndex(driverModelId);
          driver->m_nCreatedBy = 2;
          driver->m_nPedType = PED_TYPE_CIVMALE;
          driver->m_nStatus = eEntityStatus::STATUS_PHYSICS;
          CWorld::Add(driver);
          Command<Commands::WARP_CHAR_INTO_CAR>(driver, veh);
          veh->m_pDriver = driver;

          // Ensure engine and handbrake flags allow motion
          veh->bEngineOn = true;
          veh->m_nStatus = eEntityStatus::STATUS_PHYSICS;
          veh->m_nHandbrakeOn = false;

          const uint32_t compId =
              g_physicalFeedback[id].companyId.load(std::memory_order_relaxed) %
              3;
          CVector targetDest(0.0f, 0.0f, 0.0f);
          if (id < k_truckCount) {
            // Master Interstate Hauler: target from s_highwayLoop
            const uint32_t targetWpIdx =
                (directive.targetWaypointIndex + 2) % s_waypointCount;
            g_physicalFeedback[id].targetWaypointIndex.store(
                targetWpIdx, std::memory_order_release);
            const auto &targetWp = k_highwayWaypoints[targetWpIdx];
            targetDest = CVector(targetWp.x, targetWp.y, targetWp.z);
          } else {
            // Local Feeder Shuttle: target from company route
            const auto &rNodes = g_customRoutes[compId].nodes;
            if (!rNodes.empty()) {
              uint32_t targetWpIdx = directive.targetWaypointIndex;
              if (targetWpIdx >= rNodes.size())
                targetWpIdx = static_cast<uint32_t>(rNodes.size() - 1);
              g_physicalFeedback[id].targetWaypointIndex.store(
                  targetWpIdx, std::memory_order_release);
              targetDest = CVector(rNodes[targetWpIdx].x, rNodes[targetWpIdx].y,
                                   rNodes[targetWpIdx].z);
            }
          }

          // Configure autopilot:
          veh->m_autoPilot.m_nCarMission = MISSION_GOTOCOORDS;
          veh->m_autoPilot.m_nCarDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
          const bool inWageStrike =
              (s_companyBalances[compId < 3 ? compId : (id % 3)].load(
                   std::memory_order_relaxed) < 0) ||
              (s_companyStrikeTicks[compId < 3 ? compId : (id % 3)].load(
                   std::memory_order_relaxed) > 0);
          bool heavy = g_physicalFeedback[id].isOverloaded.load(
              std::memory_order_relaxed);
          veh->m_autoPilot.m_nCruiseSpeed =
              heavy ? 12 : (inWageStrike ? 10 : 22);
          veh->m_autoPilot.m_vecDestination = targetDest;
          veh->m_autoPilot.m_nStraightLineDistance =
              0; // 0 disables wall-plowing behavior

          // Create matching trailer strictly behind tractor's fifth wheel
          if (trailerModelId > 0) {
            const float trailerOffsetDist = 7.5f;
            CVector trailerPos;
            trailerPos.x = spawnPos.x - std::sin(-hRad) * trailerOffsetDist;
            trailerPos.y = spawnPos.y - std::cos(-hRad) * trailerOffsetDist;
            trailerPos.z = spawnPos.z;

            const float trailerGroundZ =
                CWorld::FindGroundZForCoord(trailerPos.x, trailerPos.y);
            if (trailerGroundZ > -100.0f) {
              trailerPos.z = trailerGroundZ + 0.2f;
            } else {
              trailerPos.z = groundZ + 0.2f;
            }

            CAutomobile *trailer = new CAutomobile(trailerModelId, 2, true);
            if (trailer) {
              trailer->m_nCreatedBy = 2;
              trailer->m_nPrimaryColor = veh->m_nPrimaryColor;
              trailer->m_nSecondaryColor = veh->m_nSecondaryColor;
              trailer->SetOrientation(0.0f, 0.0f, hRad);
              trailer->Teleport(trailerPos);
              trailer->m_fHealth = 1000.0f;
              trailer->bCanBeDamaged =
                  false; // Prevent explosion from initial joint twitch
              trailer->m_nStatus = eEntityStatus::STATUS_PHYSICS;
              // Release trailer handbrake and brakes completely:
              trailer->m_nHandbrakeOn = false;
              trailer->m_fBreakPedal = 0.0f;
              trailer->m_fGasPedal = 0.0f;

              CWorld::Add(trailer);
              trailer->UpdateRwMatrix();

              // Attach using native GTA engine physical joint:
              Command<Commands::ATTACH_TRAILER_TO_CAB>(trailer, veh);
              veh->m_pTrailer = trailer;
              trailer->m_pTractor = veh;
              s_truckTrailerHandles[id] = static_cast<uint32_t>(
                  CPools::ms_pVehiclePool->GetRef(trailer));
            }
          }

          const uint32_t handle =
              static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(veh));
          s_truckPhysicalHandles[id] = handle;

          // Mark active ONLY after complete creation, placement, and pool
          // registration
          g_physicalFeedback[id].x.store(spawnPos.x, std::memory_order_relaxed);
          g_physicalFeedback[id].y.store(spawnPos.y, std::memory_order_relaxed);
          g_physicalFeedback[id].z.store(spawnPos.z, std::memory_order_relaxed);
          g_physicalFeedback[id].heading.store(directive.heading,
                                               std::memory_order_relaxed);
          g_physicalFeedback[id].active.store(true, std::memory_order_release);

          // If spawning while already in stopped/broken down state, pull over
          // immediately
          if (g_physicalFeedback[id].brokenDown.load(
                  std::memory_order_relaxed) ||
              g_physicalFeedback[id].stopped.load(std::memory_order_relaxed)) {
            PullTruckToRoadside(veh, 3.5f);
            veh->bEngineOn = false;
            veh->m_nHandbrakeOn = true;
            veh->m_autoPilot.m_nCruiseSpeed = 0;
          }

          Logger::Log("[TruckSimulation] Materialized 3D Truck #%u (model %d, "
                      "trailer %d, driver %d, handle %u) at (%.1f, %.1f, %.1f)",
                      id, modelId, trailerModelId, driverModelId, handle,
                      spawnPos.x, spawnPos.y, spawnPos.z);
        }
      }
      break;
    }
    case AIDirectiveType::DespawnTruck: {
      const uint32_t id = directive.truckId;
      if (s_hasPendingSpawn && s_pendingSpawnDirective.truckId == id) {
        s_hasPendingSpawn = false;
        s_pendingSpawnRetries = 0;
      }
      if (id < k_totalTruckCount && CPools::ms_pVehiclePool) {
        const uint32_t handle = s_truckPhysicalHandles[id];
        if (handle != 0) {
          CVehicle *veh = CPools::ms_pVehiclePool->GetAtRef(handle);
          CPed *player = FindPlayerPed();
          if (veh && (!player || player->m_pVehicle != veh)) {
            if (veh->m_pTrailer) {
              CVehicle *tr = veh->m_pTrailer;
              Command<Commands::DETACH_TRAILER_FROM_CAB>(tr);
              veh->m_pTrailer = nullptr;
              tr->m_pTractor = nullptr;
              s_truckTrailerHandles[id] = 0;
              CWorld::Remove(tr);
              delete tr;
            } else if (s_truckTrailerHandles[id] != 0) {
              CVehicle *tr =
                  CPools::ms_pVehiclePool->GetAtRef(s_truckTrailerHandles[id]);
              if (tr) {
                Command<Commands::DETACH_TRAILER_FROM_CAB>(tr);
                tr->m_pTractor = nullptr;
                CWorld::Remove(tr);
                delete tr;
              }
              s_truckTrailerHandles[id] = 0;
            }
            if (veh->m_pDriver) {
              CWorld::Remove(veh->m_pDriver);
              delete veh->m_pDriver;
              veh->m_pDriver = nullptr;
            }
            CWorld::Remove(veh);
            delete veh;
            Logger::Log(
                "[TruckSimulation] Dematerialized Truck #%u (handle %u)", id,
                handle);
          }
          s_truckPhysicalHandles[id] = 0;
        }
        if (s_truckTrailerHandles[id] != 0) {
          CVehicle *tr =
              CPools::ms_pVehiclePool->GetAtRef(s_truckTrailerHandles[id]);
          if (tr) {
            Command<Commands::DETACH_TRAILER_FROM_CAB>(tr);
            tr->m_pTractor = nullptr;
            CWorld::Remove(tr);
            delete tr;
          }
          s_truckTrailerHandles[id] = 0;
        }
        // Immediately clear active physical state
        g_physicalFeedback[id].active.store(false, std::memory_order_release);
        g_physicalFeedback[id].isSpawned.store(false, std::memory_order_release);
      }
      break;
    }
    case AIDirectiveType::DisplayHudNotice: {
      if (directive.noticeMsg[0] != '\0') {
        CHud::SetHelpMessage(directive.noticeMsg, true, false, false);
      }
      break;
    }
    case AIDirectiveType::InterceptTarget: {
      char hudMsg[128];
      snprintf(hudMsg, sizeof(hudMsg),
               "~y~AI Directive #%u:~w~ Intercept (%.0f, %.0f, %.0f)",
               directive.directiveId, directive.targetX, directive.targetY,
               directive.targetZ);
      CHud::SetHelpMessage(hudMsg, true, false, false);
      break;
    }
    }
  }
}

void UpdatePhysicalTrucks(CPed *player) {
  ENGINE_TRACE("ECONOMY", "UpdatePhysicalTrucks: player=%p", player);
  if (!CPools::ms_pVehiclePool)
    return;

  for (size_t i = 0; i < k_totalTruckCount; ++i) {
    const uint32_t handle = s_truckPhysicalHandles[i];
    if (handle != 0) {
      CVehicle *veh = CPools::ms_pVehiclePool->GetAtRef(handle);
      if (veh) {
        const CVector &vpos = veh->GetPosition();
        float vheading = veh->GetHeading() * (180.0f / 3.14159265358979323846f);
        while (vheading < 0.0f)
          vheading += 360.0f;
        while (vheading >= 360.0f)
          vheading -= 360.0f;

        g_physicalFeedback[i].x.store(vpos.x, std::memory_order_relaxed);
        g_physicalFeedback[i].y.store(vpos.y, std::memory_order_relaxed);
        g_physicalFeedback[i].z.store(vpos.z, std::memory_order_relaxed);
        g_physicalFeedback[i].heading.store(vheading,
                                            std::memory_order_relaxed);
        g_physicalFeedback[i].speed.store(veh->m_vecMoveSpeed.Magnitude(),
                                          std::memory_order_relaxed);
        g_physicalFeedback[i].active.store(true, std::memory_order_release);
        g_physicalFeedback[i].isSpawned.store(true, std::memory_order_release);
        g_physicalFeedback[i].lastUpdateMs.store(CTimer::m_snTimeInMilliseconds,
                                                 std::memory_order_relaxed);

        // Synchronous Roadside Pull-Over Request Handler (Resting, Breakdown,
        // or Inspection)
        if (g_physicalFeedback[i].pullOverRequested.exchange(
                false, std::memory_order_acq_rel)) {
          PullTruckToRoadside(reinterpret_cast<CAutomobile *>(veh), 3.5f);
          veh->bEngineOn = false;
          veh->m_nHandbrakeOn = true;
          veh->m_autoPilot.m_nCruiseSpeed = 0;
        }

        // Detect Player Hijacking:
        if (player && player->m_pVehicle == veh) {
          if (player->m_pPlayerData && player->m_pPlayerData->m_pWanted) {
            if (player->m_pPlayerData->m_pWanted->m_nWantedLevel < 2) {
              Command<Commands::ALTER_WANTED_LEVEL>(0, 2);
            }
          }

          const auto &sfDepotWp = k_highwayWaypoints[240];
          const float sfDx = vpos.x - sfDepotWp.x;
          const float sfDy = vpos.y - sfDepotWp.y;
          if ((sfDx * sfDx + sfDy * sfDy) < (25.0f * 25.0f)) {
            const uint32_t cargoWeight =
                g_physicalFeedback[i].cargoWeightTons.load(
                    std::memory_order_relaxed);
            const uint8_t compId =
                g_physicalFeedback[i].companyId.load(std::memory_order_relaxed);

            if (player->m_pPlayerData) {
              player->m_pPlayerData->m_nMoney +=
                  static_cast<int>(cargoWeight * 250);
            }
            CHud::SetHelpMessage(
                "~g~HIJACK REWARD: Delivered stolen freight for cash!~w~", true,
                false, false);

            // Native air-brake hiss sound opcode
            Command<Commands::REPORT_MISSION_AUDIO_EVENT_AT_POSITION>(
                sfDepotWp.x, sfDepotWp.y, sfDepotWp.z, 0x1058);
            Command<Commands::PLAY_MISSION_AUDIO>(1);

            // Deduct penalty from the owning company balance
            const uint64_t penalty = cargoWeight * 250;
            if (compId < 3) {
              const uint64_t curBal =
                  s_companyBalances[compId].load(std::memory_order_relaxed);
              if (curBal >= penalty) {
                s_companyBalances[compId].fetch_sub(penalty,
                                                    std::memory_order_relaxed);
              } else {
                s_companyBalances[compId].store(0, std::memory_order_relaxed);
              }
            }

            // Warp CJ safely out of the cab to ground coordinates nearby
            Command<Commands::WARP_CHAR_FROM_CAR_TO_COORD>(
                player, sfDepotWp.x + 5.0f, sfDepotWp.y + 5.0f, sfDepotWp.z);

            // Reset truck to Ocean Docks via AI Director
            g_physicalFeedback[i].hijackedReset.store(
                true, std::memory_order_release);
          }
        }

        // Roadside SOS Assistance:
        if (g_physicalFeedback[i].brokenDown.load(std::memory_order_relaxed)) {
          if (player && !player->m_pVehicle) {
            const CVector hoodPos =
                veh->TransformFromObjectSpace(CVector(0.0f, 3.5f, 0.0f));
            const CVector pPos = player->GetPosition();
            const float distToHood = (pPos - hoodPos).Magnitude();
            if (distToHood <= 4.5f) {
              CHud::SetHelpMessage("~w~Press ~y~Y ~w~to repair truck engine "
                                   "and claim ~g~$350~w~ bounty",
                                   true, false, false);
              if (GetAsyncKeyState(0x59) & 1) { // Key 'Y'
                if (player->m_pPlayerData) {
                  player->m_pPlayerData->m_nMoney += 350;
                }
                CHud::SetHelpMessage(
                    "~g~ENGINE REPAIRED! Collected $350 Bounty~w~", true, false,
                    false);
                g_physicalFeedback[i].brokenDown.store(
                    false, std::memory_order_release);
                g_physicalFeedback[i].stopped.store(false,
                                                    std::memory_order_release);
                veh->bEngineOn = true;
                veh->m_nHandbrakeOn = false;
                veh->m_autoPilot.m_nCarMission = MISSION_GOTOCOORDS;
                veh->m_autoPilot.m_nCruiseSpeed = 22;
              }
            }
          }
        }

        // Destruction check (health zero or wrecked status)
        if (veh->m_fHealth <= 0.0f ||
            veh->m_nStatus == eEntityStatus::STATUS_WRECKED) {
          g_physicalFeedback[i].destroyed.store(true,
                                                std::memory_order_release);
        }

        // Waypoint navigation check:
        if (veh->m_fHealth > 0.0f &&
            veh->m_nStatus != eEntityStatus::STATUS_WRECKED) {
          if (i < k_truckCount) {
            // Master Interstate Hauler: target current node from master highway
            // loop
            const uint32_t curWp =
                g_physicalFeedback[i].targetWaypointIndex.load(
                    std::memory_order_relaxed);
            const auto &currentWp = k_highwayWaypoints[curWp % s_waypointCount];
            const float dx = vpos.x - currentWp.x;
            const float dy = vpos.y - currentWp.y;
            if ((dx * dx + dy * dy) < (18.0f * 18.0f)) {
              if ((curWp % s_waypointCount) == 240) {
                Command<Commands::REPORT_MISSION_AUDIO_EVENT_AT_POSITION>(
                    vpos.x, vpos.y, vpos.z, 0x1058);
                Command<Commands::PLAY_MISSION_AUDIO>(1);
              }
              const uint32_t nextWpIdx = (curWp + 1) % s_waypointCount;
              g_physicalFeedback[i].targetWaypointIndex.store(
                  nextWpIdx, std::memory_order_release);
              const uint32_t lookAheadIdx = (nextWpIdx + 1) % s_waypointCount;
              const auto &targetWp = k_highwayWaypoints[lookAheadIdx];
              veh->m_autoPilot.m_vecDestination =
                  CVector(targetWp.x, targetWp.y, targetWp.z);
            }
          } else {
            // Local Feeder Shuttle: follow company custom route shuttle-style
            const uint32_t compId = g_physicalFeedback[i].companyId.load(
                                        std::memory_order_relaxed) %
                                    3;
            const auto &rNodes = g_customRoutes[compId].nodes;
            if (rNodes.size() >= 2) {
              uint32_t curWp = g_physicalFeedback[i].targetWaypointIndex.load(
                  std::memory_order_relaxed);
              if (curWp >= rNodes.size())
                curWp = static_cast<uint32_t>(rNodes.size() - 1);
              const auto &targetNode = rNodes[curWp];
              const float dx = vpos.x - targetNode.x;
              const float dy = vpos.y - targetNode.y;
              if ((dx * dx + dy * dy) < (18.0f * 18.0f)) {
                bool fwd = g_physicalFeedback[i].travelForward.load(
                    std::memory_order_relaxed);
                uint32_t nextWp = curWp;
                if (fwd) {
                  if (curWp + 1 < rNodes.size()) {
                    nextWp = curWp + 1;
                  } else {
                    // Reached Cargo Unloading Terminal (Node N-1)!
                    fwd = false;
                    nextWp = (curWp > 0) ? curWp - 1 : 0;
                    g_physicalFeedback[i].travelForward.store(
                        false, std::memory_order_release);

                    // Sound air-brakes hiss at destination terminal
                    Command<Commands::REPORT_MISSION_AUDIO_EVENT_AT_POSITION>(
                        vpos.x, vpos.y, vpos.z, 0x1058);
                    Command<Commands::PLAY_MISSION_AUDIO>(1);
                  }
                } else {
                  if (curWp > 0) {
                    nextWp = curWp - 1;
                  } else {
                    // Reached Origin Base (Node 0)!
                    fwd = true;
                    nextWp = (rNodes.size() > 1) ? 1 : 0;
                    g_physicalFeedback[i].travelForward.store(
                        true, std::memory_order_release);

                    // Sound air-brakes hiss at origin base
                    Command<Commands::REPORT_MISSION_AUDIO_EVENT_AT_POSITION>(
                        vpos.x, vpos.y, vpos.z, 0x1058);
                    Command<Commands::PLAY_MISSION_AUDIO>(1);
                  }
                }
                g_physicalFeedback[i].targetWaypointIndex.store(
                    nextWp, std::memory_order_release);
                const auto &nextNode = rNodes[nextWp];
                veh->m_autoPilot.m_vecDestination =
                    CVector(nextNode.x, nextNode.y, nextNode.z);
              }
            }
          }
        }

        // Trailer Reconnect Watchdog
        if (s_truckTrailerHandles[i] != 0) {
          CVehicle *tr =
              CPools::ms_pVehiclePool->GetAtRef(s_truckTrailerHandles[i]);
          if (tr) {
            if (veh->m_pTrailer != tr) {
              Command<Commands::ATTACH_TRAILER_TO_CAB>(tr, veh);
              veh->m_pTrailer = tr;
              tr->m_pTractor = veh;
            }
            tr->m_nHandbrakeOn = false;
            tr->m_fBreakPedal = 0.0f;
          } else {
            s_truckTrailerHandles[i] = 0;
          }
        }

        // Unconditionally keep drive train moving unless broken down or stopped
        if (veh->m_fHealth > 0.0f &&
            veh->m_nStatus != eEntityStatus::STATUS_WRECKED) {
          if (player && player->m_pVehicle == veh) {
            // Player is manually driving the hijacked truck
            veh->bEngineOn = true;
            veh->m_nHandbrakeOn = false;
          } else if (!g_physicalFeedback[i].brokenDown.load(
                         std::memory_order_relaxed) &&
                     !g_physicalFeedback[i].stopped.load(
                         std::memory_order_relaxed)) {
            veh->bEngineOn = true;
            veh->m_nHandbrakeOn = false;
            veh->m_autoPilot.m_nCarMission = MISSION_GOTOCOORDS;
            veh->m_autoPilot.m_nCarDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
            const uint32_t compId =
                g_physicalFeedback[i].companyId.load(std::memory_order_relaxed);
            const bool inWageStrike =
                (s_companyBalances[compId < 3 ? compId : (i % 3)].load(
                     std::memory_order_relaxed) < 0) ||
                (s_companyStrikeTicks[compId < 3 ? compId : (i % 3)].load(
                     std::memory_order_relaxed) > 0);
            bool heavy = g_physicalFeedback[i].isOverloaded.load(
                std::memory_order_relaxed);
            veh->m_autoPilot.m_nCruiseSpeed =
                heavy ? 12 : (inWageStrike ? 10 : 22);
            if (veh->m_pTrailer) {
              veh->m_pTrailer->m_nHandbrakeOn = false;
              veh->m_pTrailer->m_fBreakPedal = 0.0f;
            }
          } else {
            if (g_physicalFeedback[i].brokenDown.load(
                    std::memory_order_relaxed) ||
                g_physicalFeedback[i].stopped.load(std::memory_order_relaxed)) {
              veh->bEngineOn = false;
              veh->m_nHandbrakeOn = true;
            }
            veh->m_autoPilot.m_nCruiseSpeed = 0;
          }
        }
      } else {
        s_truckPhysicalHandles[i] = 0;
        s_truckTrailerHandles[i] = 0;
        g_physicalFeedback[i].active.store(false, std::memory_order_release);
        g_physicalFeedback[i].isSpawned.store(false, std::memory_order_release);
      }
    } else {
      g_physicalFeedback[i].active.store(false, std::memory_order_release);
      g_physicalFeedback[i].isSpawned.store(false, std::memory_order_release);
    }
  }
}

void CleanupAllPhysicalTrucks() {
  if (CPools::ms_pVehiclePool) {
    for (size_t i = 0; i < k_totalTruckCount; ++i) {
      const uint32_t handle = s_truckPhysicalHandles[i];
      if (handle != 0) {
        CVehicle *veh = CPools::ms_pVehiclePool->GetAtRef(handle);
        if (veh) {
          if (veh->m_pTrailer) {
            CVehicle *trailer = veh->m_pTrailer;
            Command<Commands::DETACH_TRAILER_FROM_CAB>(trailer);
            veh->m_pTrailer = nullptr;
            trailer->m_pTractor = nullptr;
            s_truckTrailerHandles[i] = 0;
            CWorld::Remove(trailer);
            delete trailer;
          } else if (s_truckTrailerHandles[i] != 0) {
            CVehicle *tr =
                CPools::ms_pVehiclePool->GetAtRef(s_truckTrailerHandles[i]);
            if (tr) {
              Command<Commands::DETACH_TRAILER_FROM_CAB>(tr);
              tr->m_pTractor = nullptr;
              CWorld::Remove(tr);
              delete tr;
            }
            s_truckTrailerHandles[i] = 0;
          }
          if (veh->m_pDriver) {
            CWorld::Remove(veh->m_pDriver);
            delete veh->m_pDriver;
            veh->m_pDriver = nullptr;
          }
          CWorld::Remove(veh);
          delete veh;
        }
        s_truckPhysicalHandles[i] = 0;
      }
      if (s_truckTrailerHandles[i] != 0) {
        CVehicle *tr =
            CPools::ms_pVehiclePool->GetAtRef(s_truckTrailerHandles[i]);
        if (tr) {
          Command<Commands::DETACH_TRAILER_FROM_CAB>(tr);
          tr->m_pTractor = nullptr;
          CWorld::Remove(tr);
          delete tr;
        }
        s_truckTrailerHandles[i] = 0;
      }
    }
  }
}
