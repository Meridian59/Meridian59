# makefile for bitmaps

!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\textures
OUTDIR = $(TOPDIR)\resource\bin

WALL = -r

.SUFFIXES: .bmp

{$(SOURCEDIR)}.bmp.bgf:
	makebgf -o $*.bgf -n "Unnamed $*" 1 $< 1 1 1

# currently fried ones
# none


BGFS = \
	grd00001.bgf \
	grd00002.bgf \
	grd00003.bgf \
	grd00010.bgf \
	grd00011.bgf \
	grd00012.bgf \
	grd00013.bgf \
	grd00014.bgf \
	grd00015.bgf \
	grd00016.bgf \
	grd00017.bgf \
	grd00018.bgf \
	grd00019.bgf \
	grd00020.bgf \
	grd00021.bgf \
	grd00022.bgf \
	grd00023.bgf \
	grd00024.bgf \
	grd00025.bgf \
	grd00026.bgf \
	grd00027.bgf \
	grd00028.bgf \
	grd00029.bgf \
	grd00030.bgf \
	grd01004.bgf \
	grd01011.bgf \
	grd01013.bgf \
	grd01014.bgf \
	grd01015.bgf \
	grd01016.bgf \
	grd01017.bgf \
	grd01018.bgf \
	grd01019.bgf \
	grd01020.bgf \
	grd01021.bgf \
	grd01022.bgf \
	grd01023.bgf \
	grd01024.bgf \
	grd01025.bgf \
	grd01030.bgf \
	grd01501.bgf \
	grd01502.bgf \
	grd01503.bgf \
	grd01601.bgf \
	grd01602.bgf \
	grd01603.bgf \
	grd01604.bgf \
	grd01605.bgf \
	grd01606.bgf \
	grd01607.bgf \
	grd01608.bgf \
	grd01701.bgf \
	grd01702.bgf \
	grd01703.bgf \
	grd01711.bgf \
	grd01712.bgf \
	grd01713.bgf \
	grd01721.bgf \
	grd01722.bgf \
	grd01801.bgf \
	grd01802.bgf \
	grd01803.bgf \
	grd01804.bgf \
	grd01807.bgf \
	grd01808.bgf \
	grd01809.bgf \
	grd01901.bgf \
	grd01902.bgf \
	grd01903.bgf \
	grd01904.bgf \
	grd01905.bgf \
	grd02001.bgf \
	grd02002.bgf \
	grd02003.bgf \
	grd02004.bgf \
	grd02005.bgf \
	grd02011.bgf \
	grd02012.bgf \
	grd02013.bgf \
	grd02014.bgf \
	grd02015.bgf \
	grd02016.bgf \
	grd02021.bgf \
	grd02022.bgf \
	grd02023.bgf \
	grd02024.bgf \
	grd02025.bgf \
	grd02031.bgf \
	grd02033.bgf \
	grd02051.bgf \
	grd02052.bgf \
	grd02053.bgf \
	grd02055.bgf \
	grd02056.bgf \
	grd02061.bgf \
	grd02074.bgf \
	grd02101.bgf \
	grd02103.bgf \
	grd02104.bgf \
	grd02105.bgf \
	grd02121.bgf \
	grd02122.bgf \
	grd02123.bgf \
	grd02124.bgf \
	grd02125.bgf \
	grd02126.bgf \
	grd02129.bgf \
	grd02201.bgf \
	grd02301.bgf \
	grd02302.bgf \
	grd02303.bgf \
	grd02304.bgf \
	grd02305.bgf \
	grd02306.bgf \
	grd02311.bgf \
	grd02313.bgf \
	grd02401.bgf \
	grd02402.bgf \
	grd02403.bgf \
	grd02404.bgf \
	grd02405.bgf \
	grd02501.bgf \
	grd02502.bgf \
	grd02503.bgf \
	grd02504.bgf \
	grd02505.bgf \
	grd02506.bgf \
	grd02507.bgf \
	grd02508.bgf \
	grd02509.bgf \
	grd02510.bgf \
	grd02511.bgf \
	grd02512.bgf \
	grd02513.bgf \
	grd02515.bgf \
	grd02701.bgf \
	grd02702.bgf \
	grd02801.bgf \
	grd02802.bgf \
	grd02803.bgf \
	grd02901.bgf \
	grd02903.bgf \
	grd03001.bgf \
	grd03002.bgf \
	grd03003.bgf \
	grd03004.bgf \
	grd03005.bgf \
	grd03006.bgf \
	grd03101.bgf \
	grd03102.bgf \
	grd03201.bgf \
	grd03203.bgf \
	grd03204.bgf \
	grd03301.bgf \
	grd03302.bgf \
	grd03401.bgf \
	grd03402.bgf \
	grd03403.bgf \
	grd03404.bgf \
	grd03405.bgf \
	grd03407.bgf \
	grd03408.bgf \
	grd03409.bgf \
	grd03410.bgf \
	grd03411.bgf \
	grd03412.bgf \
	grd03501.bgf \
	grd03502.bgf \
	grd03701.bgf \
	grd03702.bgf \
	grd03703.bgf \
	grd03704.bgf \
	grd03801.bgf \
	grd03802.bgf \
	grd03803.bgf \
	grd03811.bgf \
	grd03901.bgf \
	grd03902.bgf \
	grd03903.bgf \
	grd04001.bgf \
	grd04002.bgf \
	grd04003.bgf \
	grd04004.bgf \
	grd04005.bgf \
	grd04006.bgf \
	grd04101.bgf \
	grd04201.bgf \
	grd04202.bgf \
	grd04303.bgf \
	grd04304.bgf \
	grd04401.bgf \
	grd04403.bgf \
	grd04404.bgf \
	grd04406.bgf \
	grd04407.bgf \
	grd04408.bgf \
	grd04410.bgf \
	grd04501.bgf \
	grd04502.bgf \
	grd04503.bgf \
	grd04504.bgf \
	grd04505.bgf \
	grd04507.bgf \
	grd04508.bgf \
	grd04601.bgf \
	grd04602.bgf \
	grd04603.bgf \
	grd04604.bgf \
	grd04605.bgf \
	grd04606.bgf \
	grd04607.bgf \
	grd04608.bgf \
	grd04610.bgf \
	grd04701.bgf \
	grd04702.bgf \
	grd04703.bgf \
	grd04704.bgf \
	grd04705.bgf \
	grd04709.bgf \
	grd04711.bgf \
	grd04712.bgf \
	grd04713.bgf \
	grd04714.bgf \
	grd04801.bgf \
	grd04802.bgf \
	grd04803.bgf \
	grd04804.bgf \
	grd04805.bgf \
	grd04806.bgf \
	grd04807.bgf \
	grd04808.bgf \
	grd04809.bgf \
	grd04810.bgf \
	grd04811.bgf \
	grd04812.bgf \
	grd04901.bgf \
	grd05102.bgf \
	grd05201.bgf \
	grd05404.bgf \
	grd05405.bgf \
	grd05417.bgf \
	grd05418.bgf \
	grd05419.bgf \
	grd05420.bgf \
	grd05441.bgf \
	grd05504.bgf \
	grd05507.bgf \
	grd05901.bgf \
	grd05902.bgf \
	grd05903.bgf \
	grd05904.bgf \
	grd05905.bgf \
	grd05906.bgf \
	grd06001.bgf \
	grd06002.bgf \
	grd06004.bgf \
	grd06501.bgf \
	grd06502.bgf \
	grd06503.bgf \
	grd06601.bgf \
	grd06602.bgf \
	grd06603.bgf \
	grd06701.bgf \
	grd07000.bgf \
	grd07001.bgf \
	grd07101.bgf \
	grd07201.bgf \
	grd07202.bgf \
	grd07203.bgf \
	grd07204.bgf \
	grd07301.bgf \
	grd07302.bgf \
	grd07401.bgf \
	grd07501.bgf \
	grd07601.bgf \
	grd07701.bgf \
	grd07702.bgf \
	grd07801.bgf \
	grd07802.bgf \
	grd08001.bgf \
	grd08002.bgf \
	grd08003.bgf \
	grd08004.bgf \
	grd08005.bgf \
	grd08006.bgf \
	grd08101.bgf \
	grd08102.bgf \
	grd08103.bgf \
	grd08104.bgf \
	grd08105.bgf \
	grd08106.bgf \
	grd08107.bgf \
	grd08108.bgf \
	grd08401.bgf \
	grd08501.bgf \
	grd08601.bgf \
	grd08816.bgf \
	grd08817.bgf \
	grd08818.bgf \
	grd08819.bgf \
	grd08820.bgf \
	grd08821.bgf \
	grd08822.bgf \
	grd08823.bgf \
	grd08824.bgf \
	grd08825.bgf \
	grd08840.bgf \
	grd08841.bgf \
	grd08842.bgf \
	grd08843.bgf \
	grd08844.bgf \
	grd08845.bgf \
	grd08846.bgf \
	grd08847.bgf \
	grd08848.bgf \
	grd08849.bgf \
	grd08850.bgf \
	grd08851.bgf \
	grd08852.bgf \
	grd08853.bgf \
	grd08854.bgf \
	grd08855.bgf \
	grd08856.bgf \
	grd08857.bgf \
	grd08859.bgf \
	grd08860.bgf \
	grd08861.bgf \
	grd08862.bgf \
	grd08863.bgf \
	grd08864.bgf \
	grd08865.bgf \
	grd08866.bgf \
	grd08867.bgf \
	grd08868.bgf \
	grd08869.bgf \
	grd08870.bgf \
	grd08871.bgf \
	grd08872.bgf \
	grd08873.bgf \
	grd08874.bgf \
	grd08875.bgf \
	grd08876.bgf \
	grd08877.bgf \
	grd08878.bgf \
	grd08879.bgf \
	grd08880.bgf \
	grd08881.bgf \
	grd08882.bgf \
	grd08883.bgf \
	grd08884.bgf \
	grd08885.bgf \
	grd08886.bgf \
	grd08887.bgf \
	grd08888.bgf \
	grd08889.bgf \
	grd08890.bgf \
	grd08891.bgf \
	grd08892.bgf \
	grd08893.bgf \
	grd08894.bgf \
	grd08895.bgf \
	grd08896.bgf \
	grd08897.bgf \
	grd08898.bgf \
	grd08899.bgf \
	grd08900.bgf \
	grd08901.bgf \
	grd08902.bgf \
	grd08903.bgf \
	grd08904.bgf \
	grd08905.bgf \
	grd08906.bgf \
	grd08907.bgf \
	grd08908.bgf \
	grd08909.bgf \
	grd08911.bgf \
	grd08912.bgf \
	grd08913.bgf \
	grd08914.bgf \
	grd08915.bgf \
	grd08916.bgf \
	grd08917.bgf \
	grd08918.bgf \
	grd08919.bgf \
	grd08920.bgf \
	grd08921.bgf \
	grd08922.bgf \
	grd08923.bgf \
	grd08924.bgf \
	grd08925.bgf \
	grd08926.bgf \
	grd08927.bgf \
	grd08928.bgf \
	grd08929.bgf \
	grd08930.bgf \
	grd08931.bgf \
	grd08932.bgf \
	grd08933.bgf \
	grd08934.bgf \
	grd08935.bgf \
	grd08936.bgf \
	grd08937.bgf \
	grd08938.bgf \
	grd08939.bgf \
	grd08940.bgf \
	grd08941.bgf \
	grd08942.bgf \
	grd08943.bgf \
	grd08944.bgf \
	grd08945.bgf \
	grd08946.bgf \
	grd08947.bgf \
	grd08948.bgf \
	grd08949.bgf \
	grd08950.bgf \
	grd08951.bgf \
	grd08952.bgf \
	grd08953.bgf \
	grd08954.bgf \
	grd08955.bgf \
	grd08956.bgf \
	grd08957.bgf \
	grd08958.bgf \
	grd08959.bgf \
	grd08960.bgf \
	grd08961.bgf \
	grd08962.bgf \
	grd08963.bgf \
	grd08964.bgf \
	grd08965.bgf \
	grd08966.bgf \
	grd08967.bgf \
	grd08968.bgf \
	grd08969.bgf \
	grd08970.bgf \
	grd08971.bgf \
	grd08972.bgf \
	grd08973.bgf \
	grd08974.bgf \
	grd08975.bgf \
	grd08976.bgf \
	grd08977.bgf \
	grd08978.bgf \
	grd08979.bgf \
	grd08980.bgf \
	grd08981.bgf \
	grd08982.bgf \
	grd08983.bgf \
	grd08984.bgf \
	grd08985.bgf \
	grd08986.bgf \
	grd08987.bgf \
	grd08988.bgf \
	grd08989.bgf \
	grd08990.bgf \
	grd08991.bgf \
	grd08992.bgf \
	grd08993.bgf \
	grd08994.bgf \
	grd08995.bgf \
	grd08996.bgf \
	grd08997.bgf \
	grd08998.bgf \
	grd08999.bgf \
	grd09000.bgf \
	grd09001.bgf \
	grd09002.bgf \
	grd09003.bgf \
	grd09004.bgf \
	grd09005.bgf \
	grd09006.bgf \
	grd09007.bgf \
	grd09008.bgf \
	grd09009.bgf \
	grd09010.bgf \
	grd09011.bgf \
	grd09012.bgf \
	grd09013.bgf \
	grd09014.bgf \
	grd09015.bgf \
	grd09016.bgf \
	grd09017.bgf \
	grd09018.bgf \
	grd09019.bgf \
	grd09020.bgf \
	grd09021.bgf \
	grd09030.bgf \
	grd09031.bgf \
	grd09032.bgf \
	grd09033.bgf \
	grd09034.bgf \
	grd09035.bgf \
	grd09036.bgf \
	grd09037.bgf \
	grd09066.bgf \
	grd09067.bgf \
	grd09068.bgf \
	grd09069.bgf \
	grd09070.bgf \
	grd09071.bgf \
	grd09072.bgf \
	grd09073.bgf \
	grd09074.bgf \
	grd09075.bgf \
	grd09076.bgf \
	grd09077.bgf \
	grd09078.bgf \
	grd09079.bgf \
	grd09080.bgf \
	grd09081.bgf \
	grd09082.bgf \
	grd09083.bgf \
	grd09084.bgf \
	grd09085.bgf \
	grd09086.bgf \
	grd09087.bgf \
	grd09088.bgf \
	grd09089.bgf \
	grd09090.bgf \
	grd09091.bgf \
	grd09092.bgf \
	grd09093.bgf \
	grd09094.bgf \
	grd09095.bgf \
	grd09096.bgf \
	grd09097.bgf \
	grd09098.bgf \
	grd09099.bgf \
	grd09100.bgf \
	grd09104.bgf \
	grd09105.bgf \
	grd09106.bgf \
	grd09107.bgf \
	grd09108.bgf \
	grd09109.bgf \
	grd09115.bgf \
	grd09116.bgf \
	grd09117.bgf \
	grd09118.bgf \
	grd09119.bgf \
	grd09120.bgf \
	grd09121.bgf \
	grd09122.bgf \
	grd09123.bgf \
	grd09124.bgf \
	grd09125.bgf \
	grd09126.bgf \
	grd09127.bgf \
	grd09128.bgf \
	grd09129.bgf \
	grd09130.bgf \
	grd09131.bgf \
	grd09132.bgf \
	grd09133.bgf \
	grd09134.bgf \
	grd09135.bgf \
	grd09136.bgf \
	grd09137.bgf \
	grd09138.bgf \
	grd09139.bgf \
	grd09140.bgf \
	grd09141.bgf \
	grd09142.bgf \
	grd09143.bgf \
	grd09144.bgf \
	grd09149.bgf \
	grd09151.bgf \
	grd09152.bgf \
	grd09153.bgf \
	grd09154.bgf \
	grd09155.bgf \
	grd09164.bgf \
	grd09165.bgf \
	grd09166.bgf \
	grd09167.bgf \
	grd09168.bgf \
	grd09169.bgf \
	grd09170.bgf \
	grd09171.bgf \
	grd09172.bgf \
	grd09173.bgf \
	grd09174.bgf \
	grd09175.bgf \
	grd09176.bgf \
	grd09177.bgf \
	grd09178.bgf \
	grd09190.bgf \
	grd09191.bgf \
	grd09192.bgf \
	grd09193.bgf \
	grd09194.bgf \
	grd09195.bgf \
	grd09196.bgf \
	grd09197.bgf \
	grd09198.bgf \
	grd09199.bgf \
	grd09200.bgf \
	grd09201.bgf \
	grd09202.bgf \
	grd09211.bgf \
	grd09212.bgf \
	grd09213.bgf \
	grd09214.bgf \
	grd09215.bgf \
	grd09216.bgf \
	grd09217.bgf \
	grd09218.bgf \
	grd09219.bgf \
	grd09220.bgf \
	grd09221.bgf \
	grd09222.bgf \
	grd09223.bgf \
	grd09224.bgf \
	grd09225.bgf \
	grd09226.bgf \
	grd09227.bgf \
	grd09228.bgf \
	grd09237.bgf \
	grd09238.bgf \
	grd09239.bgf \
	grd09240.bgf \
	grd09241.bgf \
	grd09242.bgf \
	grd09243.bgf \
	grd09244.bgf \
	grd09245.bgf \
	grd09246.bgf \
	grd09247.bgf \
	grd09248.bgf \
	grd09249.bgf \
	grd09250.bgf \
	grd09251.bgf \
	grd09252.bgf \
	grd09253.bgf \
	grd09254.bgf \
	grd09255.bgf \
	grd09256.bgf \
	grd09257.bgf \
	grd09258.bgf \
	grd09259.bgf \
	grd09260.bgf \
	grd09261.bgf \
	grd09262.bgf \
	grd09263.bgf \
	grd09264.bgf \
	grd09265.bgf \
	grd09266.bgf \
	grd09267.bgf \
	grd09268.bgf \
	grd09269.bgf \
	grd09270.bgf \
	grd09271.bgf \
	grd09272.bgf \
	grd09273.bgf \
	grd09274.bgf \
	grd09275.bgf \
	grd09276.bgf \
	grd09277.bgf \
	grd09278.bgf \
	grd09279.bgf \
	grd09280.bgf \
	grd09281.bgf \
	grd09282.bgf \
	grd09288.bgf \
	grd09289.bgf \
	grd09290.bgf \
	grd09291.bgf \
	grd09292.bgf \
	grd09293.bgf \
	grd09294.bgf \
	grd09295.bgf \
	grd09296.bgf \
	grd09297.bgf \
	grd09298.bgf \
	grd09299.bgf \
	grd09300.bgf \
	grd09301.bgf \
	grd09302.bgf \
	grd09303.bgf \
	grd09304.bgf \
	grd09305.bgf \
	grd09306.bgf \
	grd09307.bgf \
	grd09308.bgf \
	grd09309.bgf \
	grd09310.bgf \
	grd09311.bgf \
	grd09312.bgf \
	grd09313.bgf \
	grd09314.bgf \
	grd09315.bgf \
	grd09316.bgf \
	grd09317.bgf \
	grd09318.bgf \
	grd09319.bgf \
	grd09320.bgf \
	grd09321.bgf \
	grd09322.bgf \
	grd09323.bgf \
	grd09324.bgf \
	grd09325.bgf \
	grd09326.bgf \
	grd09327.bgf \
	grd09328.bgf \
	grd09329.bgf \
	grd09330.bgf \
	grd09331.bgf \
	grd09332.bgf \
	grd09333.bgf \
	grd09334.bgf \
	grd09335.bgf \
	grd09336.bgf \
	grd09337.bgf \
	grd09338.bgf \
	grd09339.bgf \
	grd09340.bgf \
	grd09341.bgf \
	grd09342.bgf \
	grd09343.bgf \
	grd09344.bgf \
	grd09345.bgf \
	grd09346.bgf \
	grd09347.bgf \
	grd09348.bgf \
	grd09349.bgf \
	grd09350.bgf \
	grd09351.bgf \
	grd09352.bgf \
	grd09353.bgf \
	grd09354.bgf \
	grd09355.bgf \
	grd09356.bgf \
	grd09357.bgf \
	grd09358.bgf \
	grd09359.bgf \
	grd09360.bgf \
	grd09361.bgf \
	grd09362.bgf \
	grd09363.bgf \
	grd09364.bgf \
	grd09365.bgf \
	grd09366.bgf \
	grd09367.bgf \
	grd09368.bgf \
	grd09369.bgf \
	grd09370.bgf \
	grd09371.bgf \
	grd09372.bgf \
	grd09373.bgf \
	grd09374.bgf \
	grd09375.bgf \
	grd09376.bgf \
	grd09377.bgf \
	grd09378.bgf \
	grd09379.bgf \
	grd09380.bgf \
	grd09381.bgf \
	grd09382.bgf \
	grd09383.bgf \
	grd09384.bgf \
	grd09385.bgf \
	grd09386.bgf \
	grd09387.bgf \
	grd09388.bgf \
	grd09389.bgf \
	grd09390.bgf \
	grd09391.bgf \
	grd09392.bgf \
	grd09393.bgf \
	grd09394.bgf \
	grd09395.bgf \
	grd09396.bgf \
	grd09397.bgf \
	grd09398.bgf \
	grd09399.bgf \
	grd09400.bgf \
	grd09401.bgf \
	grd09402.bgf \
	grd09403.bgf \
	grd09404.bgf \
	grd09405.bgf \
	grd09406.bgf \
	grd09407.bgf \
	grd09408.bgf \
	grd09409.bgf \
	grd09410.bgf \
	grd09411.bgf \
	grd09412.bgf \
	grd09413.bgf \
	grd09414.bgf \
	grd09415.bgf \
	grd09416.bgf \
	grd09417.bgf \
	grd09418.bgf \
	grd09419.bgf \
	grd09420.bgf \
	grd09423.bgf \
	grd09424.bgf \
	grd09425.bgf \
	grd09426.bgf \
	grd09427.bgf \
	grd09428.bgf \
	grd09503.bgf \
	grd09504.bgf \
	grd09505.bgf \
	grd09506.bgf \
	grd09507.bgf \
	grd09508.bgf \
	grd09509.bgf \
	grd09510.bgf \
	grd09511.bgf \
	grd09513.bgf \
	grd09514.bgf \
	grd09515.bgf \
	grd09516.bgf \
	grd09517.bgf \
	grd09518.bgf \
	grd09519.bgf \
	grd09520.bgf \
	grd09521.bgf \
	grd09522.bgf \
	grd09523.bgf \
	grd09524.bgf \
	grd09525.bgf \
	grd09526.bgf \
	grd09527.bgf \
	grd09528.bgf \
	grd09529.bgf \
	grd09530.bgf \
	grd09531.bgf \
	grd09532.bgf \
	grd09533.bgf \
	grd09534.bgf \
	grd09535.bgf \
	grd09536.bgf \
	grd09537.bgf \
	grd09538.bgf \
	grd09539.bgf \
	grd09540.bgf \
	grd09541.bgf \
	grd09542.bgf \
	grd09543.bgf \
	grd09544.bgf \
	grd09545.bgf \
	grd09546.bgf \
	grd09548.bgf \
	grd09549.bgf \
	grd09550.bgf \
	grd09551.bgf \
	grd09552.bgf \
	grd09553.bgf \
	grd09554.bgf \
	grd09555.bgf \
	grd09556.bgf \
	grd09557.bgf \
	grd09558.bgf \
	grd09559.bgf \
	grd09560.bgf \
	grd09561.bgf \
	grd09562.bgf \
	grd09563.bgf \
	grd09564.bgf \
	grd09565.bgf \
	grd09566.bgf \
	grd09567.bgf \
	grd09568.bgf \
	grd09569.bgf \
	grd09570.bgf \
	grd09571.bgf \
	grd09572.bgf \
	grd09573.bgf \
	grd09574.bgf \
	grd09575.bgf \
	grd09576.bgf \
	grd09577.bgf \
	grd09578.bgf \
	grd09579.bgf \
	grd09580.bgf \
	grd09581.bgf \
	grd09582.bgf \
	grd09583.bgf \
	grd09584.bgf \
	grd09585.bgf \
	grd09586.bgf \
	grd09587.bgf \
	grd09588.bgf \
	grd09589.bgf \
	grd09590.bgf \
	grd09591.bgf \
	grd09592.bgf \
	grd09593.bgf \
	grd09594.bgf \
	grd09595.bgf \
	grd09596.bgf \
	grd09597.bgf \
	grd09598.bgf \
	grd09599.bgf \
	grd09600.bgf \
	grd09601.bgf \
	grd09602.bgf \
	grd09603.bgf \
	grd09604.bgf \
	grd09605.bgf \
	grd09606.bgf \
	grd09607.bgf \
	grd09608.bgf \
	grd09609.bgf \
	grd09610.bgf \
	grd09611.bgf \
	grd09612.bgf \
	grd09613.bgf \
	grd09614.bgf \
	grd09615.bgf \
	grd09616.bgf \
	grd09617.bgf \
	grd09618.bgf \
	grd09619.bgf \
	grd09620.bgf \
	grd09621.bgf \
	grd09622.bgf \
	grd09623.bgf \
	grd09624.bgf \
	grd09625.bgf \
	grd09626.bgf \
	grd09627.bgf \
	grd09628.bgf \
	grd09629.bgf \
	grd09630.bgf \
	grd09631.bgf \
	grd09632.bgf \
	grd09633.bgf \
	grd09634.bgf \
	grd09635.bgf \
	grd09636.bgf \
	grd09637.bgf \
	grd09638.bgf \
	grd09639.bgf \
	grd09640.bgf \
	grd09641.bgf \
	grd09642.bgf \
	grd09643.bgf \
	grd09644.bgf \
	grd09645.bgf \
	grd09646.bgf \
	grd09647.bgf \
	grd09648.bgf \
	grd09649.bgf \
	grd09650.bgf \
	grd09651.bgf \
	grd09652.bgf \
	grd09653.bgf \
	grd09654.bgf \
	grd09655.bgf \
	grd09656.bgf \
	grd09657.bgf \
	grd09658.bgf \
	grd09659.bgf \
	grd09660.bgf \
	grd09661.bgf \
	grd09662.bgf \
	grd09663.bgf \
	grd09664.bgf \
	grd09665.bgf \
	grd09666.bgf \
	grd09667.bgf \
	grd09668.bgf \
	grd09669.bgf \
	grd09670.bgf \
	grd09671.bgf \
	grd09672.bgf \
	grd09673.bgf \
	grd09674.bgf \
	grd09675.bgf \
	grd09676.bgf \
	grd09677.bgf \
	grd09678.bgf \
	grd09679.bgf \
	grd09680.bgf \
	grd09681.bgf \
	grd09682.bgf \
	grd09683.bgf \
	grd09684.bgf \
	grd09685.bgf \
	grd09686.bgf \
	grd09687.bgf \
	grd09688.bgf \
	grd09689.bgf \
	grd09690.bgf \
	grd09691.bgf \
	grd09692.bgf \
	grd09693.bgf \
	grd09694.bgf \
	grd09695.bgf \
	grd09696.bgf \
	grd09697.bgf \
	grd09698.bgf \
	grd09699.bgf \
	grd09700.bgf \
	grd09701.bgf \
	grd09702.bgf \
	grd09703.bgf \
	grd09704.bgf \
	grd09705.bgf \
	grd09706.bgf \
	grd09707.bgf \
	grd09708.bgf \
	grd09709.bgf \
	grd09710.bgf \
	grd09711.bgf \
	grd09712.bgf \
	grd09713.bgf \
	grd09714.bgf \
	grd09715.bgf \
	grd09716.bgf \
	grd09717.bgf \
	grd09718.bgf \
	grd09719.bgf \
	grd09720.bgf \
	grd09721.bgf \
	grd09722.bgf \
	grd09723.bgf \
	grd09724.bgf \
	grd09725.bgf \
	grd09726.bgf \
	grd09727.bgf \
	grd09734.bgf \
	grd09735.bgf \
	grd09736.bgf \
	grd09737.bgf \
	grd09738.bgf \
	grd09739.bgf \
	grd09740.bgf \
	grd09741.bgf \
	grd09742.bgf \
	grd09743.bgf \
	grd09744.bgf \
	grd09745.bgf \
	grd09746.bgf \
	grd09753.bgf \
	grd09765.bgf \
	grd09766.bgf \
	grd09767.bgf \
	grd09768.bgf \
	grd09769.bgf \
	grd09770.bgf \
	grd09771.bgf \
	grd09772.bgf \
	grd09773.bgf \
	grd09776.bgf \
	grd09777.bgf \
	grd10000.bgf \
	grd10001.bgf \
	grd10002.bgf \
	grd10003.bgf \
	grd10004.bgf \
	grd10005.bgf \
	grd10006.bgf \
	grd10007.bgf \
	grd10008.bgf \
	grd10009.bgf \
	grd10010.bgf \
	grd10011.bgf \
	grd10012.bgf \
	grd10013.bgf \
	grd10014.bgf \
	grd10020.bgf \
	grd10021.bgf \
	grd10022.bgf \
	grd10023.bgf \
	grd10030.bgf \
	grd10031.bgf \
	grd10032.bgf \
	grd10033.bgf \
	grd10040.bgf \
	grd10041.bgf \
	grd10042.bgf \
	grd10043.bgf \
	grd10044.bgf \
	grd10045.bgf \
	grd10046.bgf \
	grd10047.bgf \
	grd10050.bgf \
	grd10051.bgf \
	grd10052.bgf \
	grd10053.bgf \
	grd10054.bgf \
	grd10060.bgf \
	grd10061.bgf \
	grd10062.bgf \
	grd10063.bgf \
	grd10064.bgf \
	grd10065.bgf \
	grd10070.bgf \
	grd10071.bgf \
	grd10072.bgf \
	grd10073.bgf \
	grd10074.bgf \
	grd10080.bgf \
	grd10081.bgf \
	grd10090.bgf \
	grd10093.bgf \
	grd10094.bgf \
	grd10095.bgf \
	grd10096.bgf \
	grd10097.bgf \
	grd10100.bgf \
	grd10101.bgf \
	grd10102.bgf \
	grd10103.bgf \
	grd10104.bgf \
	grd10110.bgf \
	grd10111.bgf \
	grd10120.bgf \
	grd10121.bgf \
	grd10130.bgf \
	grd10131.bgf \
	grd10132.bgf \
	grd10140.bgf \
	grd10141.bgf \
	grd10142.bgf \
	grd10143.bgf \
	grd10150.bgf \
	grd10151.bgf \
	grd10152.bgf \
	grd10153.bgf \
	grd10170.bgf \
	grd10171.bgf \
	grd10172.bgf \
	grd10173.bgf \
	grd10174.bgf \
	grd10180.bgf \
	grd10181.bgf \
	grd10182.bgf \
	grd10183.bgf \
	grd10190.bgf \
	grd10191.bgf \
	grd10194.bgf \
	grd10200.bgf \
	grd10201.bgf \
	grd10202.bgf \
	grd10203.bgf \
	grd10204.bgf \
	grd10300.bgf \
	grd10301.bgf \
	grd10302.bgf \
	grd10303.bgf \
	grd11000.bgf \
	grd11001.bgf \
	grd11002.bgf \
	grd11003.bgf \
	grd11004.bgf \
	grd11005.bgf \
	grd11006.bgf \
	grd11007.bgf \
	grd11008.bgf \
	grd11009.bgf \
	grd11010.bgf \
	grd11011.bgf \
	grd11012.bgf \
	grd11013.bgf \
	grd11014.bgf \
	grd11015.bgf \
	grd11016.bgf \
	grd11017.bgf \
	grd11018.bgf \
	grd11019.bgf \
	grd11020.bgf \
	grd11021.bgf \
	grd11022.bgf \
	grd11023.bgf \
	grd11024.bgf \
	grd11025.bgf \
	grd11026.bgf \
	grd11027.bgf \
	grd11028.bgf \
	grd11029.bgf \
	grd11030.bgf \
	grd11031.bgf \
	grd11032.bgf \
	grd11033.bgf \
	grd11034.bgf \
	grd11035.bgf \
	grd11036.bgf \
	grd11037.bgf \
	grd11038.bgf \
	grd11039.bgf \
	grd11040.bgf \
	grd11041.bgf \
	grd11042.bgf \
	grd11043.bgf \
	grd11050.bgf \
	grd11051.bgf \
	grd11052.bgf \
	grd11053.bgf \
	grd11054.bgf \
	grd11055.bgf \
	grd11056.bgf \
	grd11057.bgf \
	grd11058.bgf \
	grd11059.bgf \
	grd11060.bgf \
	grd11061.bgf \
	grd11062.bgf \
	grd11063.bgf \
	grd11064.bgf \
	grd11065.bgf \
	grd11066.bgf \
	grd11067.bgf \
	grd11068.bgf \
	grd11069.bgf \
	grd11070.bgf \
	grd11071.bgf \
	grd11072.bgf \
	grd11073.bgf \
	grd11074.bgf \
	grd11075.bgf \
	grd11076.bgf \
	grd11077.bgf \
	grd11078.bgf \
	grd11079.bgf \
	grd11080.bgf \
	grd11081.bgf \
	grd11082.bgf \
	grd11083.bgf \
	grd11084.bgf \
	grd11085.bgf \
	grd11086.bgf \
	grd11087.bgf \
	grd11088.bgf \
	grd11089.bgf \
	grd11090.bgf \
	grd11091.bgf \
	grd11092.bgf \
	grd11093.bgf \
	grd11094.bgf \
	grd11095.bgf \
	grd11096.bgf \
	grd11097.bgf \
	grd11098.bgf \
	grd11099.bgf \
	grd11100.bgf \
	grd11101.bgf \
	grd11102.bgf \
	grd11103.bgf \
	grd11104.bgf \
	grd11105.bgf \
	grd11106.bgf \
	grd11107.bgf \
	grd11108.bgf \
	grd11109.bgf \
	grd11110.bgf \
	grd11111.bgf \
	grd11112.bgf \
	grd11113.bgf \
	grd11125.bgf \
	grd11126.bgf \
	grd11127.bgf \
	grd11128.bgf \
	grd11129.bgf \
	grd11130.bgf \
	grd11131.bgf \
	grd11132.bgf \
	grd11133.bgf \
	grd11134.bgf \
	grd11135.bgf \
	grd11136.bgf \
	grd11137.bgf \
	grd11138.bgf \
	grd11139.bgf \
	grd11140.bgf \
	grd11141.bgf \
	grd11142.bgf \
	grd11143.bgf \
	grd11144.bgf \
	grd11145.bgf \
	grd11146.bgf \
	grd11147.bgf \
	grd11148.bgf \
	grd11149.bgf \
	grd11150.bgf \
	grd11151.bgf \
	grd11152.bgf \
	grd11153.bgf \
	grd11154.bgf \
	grd11155.bgf \
	grd11156.bgf \
	grd11157.bgf \
	grd11158.bgf \
	grd11159.bgf \
	grd11160.bgf \
	grd11161.bgf \
	grd11162.bgf \
	grd11163.bgf \
	grd11164.bgf \
	grd11165.bgf \
	grd11166.bgf \
	grd11167.bgf \
	grd11168.bgf \
	grd11169.bgf \
	grd11170.bgf \
	grd11171.bgf \
	grd11172.bgf \
	grd11173.bgf \
	grd11174.bgf \
	grd11175.bgf \
	grd11176.bgf \
	grd50002.bgf \
	grd50003.bgf \
	grd50004.bgf \
	grd50005.bgf \
	grd50050.bgf \
	grd50051.bgf \
	grd50052.bgf \
	grd50053.bgf \
	grd50054.bgf \
	grd50055.bgf \
	grd50056.bgf \
	grd50057.bgf \
	grd50058.bgf \
	grd50059.bgf \
	grd50060.bgf \
	grd50061.bgf \
	grd50062.bgf \
	grd50063.bgf \
	grd50064.bgf \
	grd50065.bgf \
	grd50066.bgf \
	grd50067.bgf \
	grd50068.bgf \
	grd50069.bgf \
	grd50070.bgf \
	grd50071.bgf \
	grd50072.bgf \
	grd50073.bgf \
	grd50074.bgf \
	grd50075.bgf \
	grd50076.bgf \
	grd50077.bgf \

#cmk unknown	grd11044.bgf \

all:  $(BGFS)

# bitmaps for walls, ceilings, and floors

grd00001.bgf:
	makebgf -o $*.bgf $(WALL) -n "Brick Blue" 1 $(SOURCEDIR)\walltex.bmp 1 1 1

grd00002.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick old stone" -s 2 1 $(SOURCEDIR)\stonwltx.bmp 1 1 1

grd00003.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick old stone door" -s 2 1 $(SOURCEDIR)\stonwldr.bmp 1 1 1

grd00010.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web Corner 64x64/128" -s 2 1 $(SOURCEDIR)\00010w2.bmp 1 1 1

grd00011.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web Corner 32x64/128" -s 2 1 $(SOURCEDIR)\00011w2.bmp 1 1 1

grd00012.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web Corner 64x64 Hang" -s 2 1 $(SOURCEDIR)\00012w2.bmp 1 1 1

grd00013.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web Corner 32x64 Hang" -s 2 1 $(SOURCEDIR)\00013w2.bmp 1 1 1

grd00014.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web Corner 64x32/128" -s 2 1 $(SOURCEDIR)\00014w2.bmp 1 1 1

grd00015.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web Corner 64x32 Hang" -s 2 1 $(SOURCEDIR)\00015w2.bmp 1 1 1

grd00016.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Crate (64x64) Greenwich" -s 1 1 $(SOURCEDIR)\00016w1.bmp 1 1 1

grd00017.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 17" -s 2 1 $(SOURCEDIR)\00017w2.bmp 1 1 1

grd00018.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 18" -s 2 1 $(SOURCEDIR)\00018w2.bmp 1 1 1

grd00019.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 19" -s 2 1 $(SOURCEDIR)\00019w2.bmp 1 1 1

grd00020.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Dark St Andrews X" -s 2 1 $(SOURCEDIR)\00020w2.bmp 1 1 1

grd00021.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Dark Shackles" -s 2 1 $(SOURCEDIR)\00021w2.bmp 1 1 1

grd00022.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 22" -s 2 1 $(SOURCEDIR)\00022w2.bmp 1 1 1

grd00023.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 23" -s 2 1 $(SOURCEDIR)\00023w2.bmp 1 1 1

grd00024.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 24" -s 2 1 $(SOURCEDIR)\00024w2.bmp 1 1 1

grd00025.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 25" -s 2 1 $(SOURCEDIR)\00025w2.bmp 1 1 1

grd00026.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 26" -s 2 1 $(SOURCEDIR)\00026w2.bmp 1 1 1

grd00027.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 27" -s 2 1 $(SOURCEDIR)\00027w2.bmp 1 1 1

grd00028.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reserved 28" -s 2 1 $(SOURCEDIR)\00028w2.bmp 1 1 1

grd00029.bgf:
	 makebgf -o $*.bgf $(WALL) -n "MipMap Test" -s 1 4 $(SOURCEDIR)\00029w1a.bmp $(SOURCEDIR)\00029w1b.bmp $(SOURCEDIR)\00029w1c.bmp $(SOURCEDIR)\00029w1d.bmp 1 4 1 2 3 4

grd00030.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cage" -s 2 2 $(SOURCEDIR)\cagetex.bmp $(SOURCEDIR)\blank.bmp 2 1 1 1 2

grd01004.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Kitchen Red Tile" 1 $(SOURCEDIR)\tiletex.bmp 1 1 1

grd01011.bgf:
	 makebgf -o $*.bgf -n "Cave Ceiling (too griddy)" 1 $(SOURCEDIR)\ceilitex.bmp 1 1 1

grd01013.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Mouth Left" -s 2 1 $(SOURCEDIR)\cavmLtex.bmp 1 1 1

grd01014.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Mouth Right" -s 2 1 $(SOURCEDIR)\cavmRtex.bmp 1 1 1

grd01015.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Diamond" 1 $(SOURCEDIR)\mtiletex.bmp 1 1 1

grd01016.bgf:
	 makebgf -o $*.bgf -n "Carpet Red Right" 1 $(SOURCEDIR)\rdcartex.bmp 1 1 1

grd01017.bgf:
	 makebgf -o $*.bgf -n "Carpet Red Left" 1 $(SOURCEDIR)\rdcatex2.bmp 1 1 1

grd01018.bgf:
	 makebgf -o $*.bgf -n "Carpet Red Middle" 1 $(SOURCEDIR)\rdcatex3.bmp 1 1 1

grd01019.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Purple Plain" -s 2 1 $(SOURCEDIR)\prtapest.bmp 1 1 1

grd01020.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Purple with Crown" -s 2 1 \
		 $(SOURCEDIR)\prtaphed.bmp 1 1 1

grd01021.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Forest Deep 1" 1 $(SOURCEDIR)\forsttex.bmp 1 1 1

grd01022.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Solid Gray" 1 $(SOURCEDIR)\gray.bmp 1 1 1

grd01023.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door Right Gold Lion" -s 2 1 $(SOURCEDIR)\doorrite.bmp 1 1 1

grd01024.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door Left Gold Lion" -s 2 1 $(SOURCEDIR)\doorleft.bmp 1 1 1

grd01025.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Red" -s 2 1 $(SOURCEDIR)\rdtapest.bmp 1 1 1

grd01030.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Curtain Red" -s 2 1 $(SOURCEDIR)\curtntex.bmp 1 1 1

grd01501.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Forest Deep 2" -s 2 1 $(SOURCEDIR)\forstex2.bmp 1 1 1

grd01502.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Forest Deep 3" -s 2 1 $(SOURCEDIR)\forstex3.bmp 1 1 1

grd01503.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Forest Deep 4" -s 2 1 $(SOURCEDIR)\forstex4.bmp 1 1 1

grd01601.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 1" -s 2 1 $(SOURCEDIR)\stontex1.bmp 1 1 1

grd01602.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 2" -s 2 1 $(SOURCEDIR)\stontex2.bmp 1 1 1

grd01603.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 3" -s 2 1 $(SOURCEDIR)\stontex3.bmp 1 1 1

grd01604.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 4" -s 2 1 $(SOURCEDIR)\stontex4.bmp 1 1 1

grd01605.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 5" -s 2 1 $(SOURCEDIR)\cavetex.bmp 1 1 1

grd01606.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Entrance Tall" -s 2 1 $(SOURCEDIR)\cavextex.bmp 1 1 1

grd01607.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 4 Floor" -s 2 1 $(SOURCEDIR)\caveflor.bmp 1 1 1

grd01608.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Cave Stone 4 Opening" -s 2 1 $(SOURCEDIR)\cavearch.bmp 1 1 1

grd01701.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble White 1" 1 $(SOURCEDIR)\mwalltex.bmp 1 1 1

grd01702.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble White 2" 1 $(SOURCEDIR)\mrbltex2.bmp 1 1 1

grd01703.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble White 3" 1 $(SOURCEDIR)\mrbltex3.bmp 1 1 1

grd01711.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Tapestry Unicorn" -s 2 1 $(SOURCEDIR)\tapestex.bmp 1 1 1

grd01712.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Tapestry Dragon" -s 2 1 $(SOURCEDIR)\tapestx2.bmp 1 1 1

grd01713.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Tapestry Cloth" -s 2 1 $(SOURCEDIR)\clothtex.bmp 1 1 1

grd01721.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Peach 1" -s 2 1 $(SOURCEDIR)\pchmrbl1.bmp 1 1 1

grd01722.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Peach 2" -s 2 1 $(SOURCEDIR)\pchmrbl2.bmp 1 1 1

grd01801.bgf:
	 makebgf -o $*.bgf -n "Grass Junky River East-West" \
	 3 \
	    $(SOURCEDIR)\rvrtex1A.bmp \
	    $(SOURCEDIR)\rvrtex1B.bmp \
	    $(SOURCEDIR)\rvrtex1C.bmp \
	 3 \
	    1 1 \
	    1 2 \
	    1 3 

grd01802.bgf:
	 makebgf -o $*.bgf -n "Water Pool" -s 2 \
	 3 \
	    $(SOURCEDIR)\poolwtrA.bmp \
	    $(SOURCEDIR)\poolwtrB.bmp \
	    $(SOURCEDIR)\poolwtrC.bmp \
	 3 \
	    1 1 \
	    1 2 \
	    1 3 

grd01803.bgf:
	 makebgf -o $*.bgf -n "Water Stream 1" -s 2 \
	 5 \
	    $(SOURCEDIR)\streamN1.bmp \
	    $(SOURCEDIR)\streamN2.bmp \
	    $(SOURCEDIR)\streamN3.bmp \
	    $(SOURCEDIR)\streamN4.bmp \
	    $(SOURCEDIR)\streamN5.bmp \
	 5 \
	    1 1 \
	    1 2 \
	    1 3 \
	    1 4 \
	    1 5 

grd01804.bgf:
	 makebgf -o $*.bgf -n "Water Stream XXX same as 1" -s 2 \
	 5 \
	    $(SOURCEDIR)\streamN1.bmp \
	    $(SOURCEDIR)\streamN2.bmp \
	    $(SOURCEDIR)\streamN3.bmp \
	    $(SOURCEDIR)\streamN4.bmp \
	    $(SOURCEDIR)\streamN5.bmp \
	 5 \
	    1 1 \
	    1 2 \
	    1 3 \
	    1 4 \
	    1 5 

grd01807.bgf:
	 makebgf -o $*.bgf -n "Water Brackish Horizontal" -s 2 \
	 3 \
	    $(SOURCEDIR)\brakwtrA.bmp \
	    $(SOURCEDIR)\brakwtrB.bmp \
	    $(SOURCEDIR)\brakwtrC.bmp \
	 3 \
	    1 1 \
	    1 2 \
	    1 3 

grd01808.bgf:
	 makebgf -o $*.bgf -n "Water Brackish Vertical" -s 2 \
	 3 \
	    $(SOURCEDIR)\brakwtrA.bmp \
	    $(SOURCEDIR)\brakwtrD.bmp \
	    $(SOURCEDIR)\brakwtrE.bmp \
	 3 \
	    1 1 \
	    1 2 \
	    1 3 

grd01809.bgf:
	 makebgf -o $*.bgf -n "River Junky East-West" \
	 3 \
	    $(SOURCEDIR)\riverA.bmp \
	    $(SOURCEDIR)\riverB.bmp \
	    $(SOURCEDIR)\riverC.bmp \
	 4 \
	    1 1 \
	    1 2 \
	    1 3 \
	    1 2 

grd01901.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fence Ornamental Tall" -s 2 1 $(SOURCEDIR)\fencetx2.bmp 1 1 1

grd01902.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fence Ornamental Gate" -s 2 1 $(SOURCEDIR)\fncgatx2.bmp 1 1 1

grd01903.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fence Ornamental" -s 2 2 $(SOURCEDIR)\fencetex.bmp $(SOURCEDIR)\blank.bmp 2 1 1 1 2

grd01904.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fence Ornamental Gate" -s 2 1 $(SOURCEDIR)\fncgatex.bmp 1 1 1

grd01905.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fence Picket" -s 1 1 $(SOURCEDIR)\piketfnc.bmp 1 1 1

grd02001.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray/Green" -s 2 1 $(SOURCEDIR)\stwaltx1.bmp 1 1 1

grd02002.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray/Green Cracked" -s 2 1 $(SOURCEDIR)\stwaltx3.bmp 1 1 1

grd02003.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray/Green Mossy" -s 2 1 $(SOURCEDIR)\stwaltx4.bmp 1 1 1

grd02004.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray/Green Door" -s 2 1 $(SOURCEDIR)\stwaldor.bmp 1 1 1

grd02005.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray/Green Floor" -s 2 1 $(SOURCEDIR)\grnfloor.bmp 1 1 1

grd02011.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray" -s 2 1 $(SOURCEDIR)\grbrktex.bmp 1 1 1

grd02012.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray Corner" -s 2 1 $(SOURCEDIR)\grcorntx.bmp 1 1 1

grd02013.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray Ivy Left" -s 2 1 $(SOURCEDIR)\grbrkivL.bmp 1 1 1

grd02014.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray Ivy Right" -s 2 1 $(SOURCEDIR)\grbrkivR.bmp 1 1 1

grd02015.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray Door" -s 2 1 $(SOURCEDIR)\grbrkdor.bmp 1 1 1

grd02016.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Gray Vertical" -s 2 1 $(SOURCEDIR)\grbrkvrt.bmp 1 1 1

grd02021.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brown/Tan" -s 2 1 $(SOURCEDIR)\orbrktex.bmp 1 1 1

grd02022.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brown/Tan Corner" -s 2 1 $(SOURCEDIR)\orcorntx.bmp 1 1 1

grd02023.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brown/Tan Ivy Left" -s 2 1 $(SOURCEDIR)\orbrkivL.bmp 1 1 1

grd02024.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brown/Tan Ivy Right" -s 2 1 $(SOURCEDIR)\orbrkivR.bmp 1 1 1

grd02025.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brown/Tan Door" -s 2 1 $(SOURCEDIR)\orbrkdor.bmp 1 1 1

grd02031.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Dark Mossy" -s 2 1 $(SOURCEDIR)\mosbrick.bmp 1 1 1

grd02033.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Dark Mossy Door" -s 2 1 $(SOURCEDIR)\msbrkdor.bmp 1 1 1

grd02051.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Blue/Gray" 1 $(SOURCEDIR)\stonAtex.bmp 1 1 1

grd02052.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Blue/Gray Cracked" 1 $(SOURCEDIR)\stonBtex.bmp 1 1 1

grd02053.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Blue/Gray Cracked Majorly" 1 $(SOURCEDIR)\stonCtex.bmp 1 1 1

grd02055.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Blue/Gray Ivy Right" 1 $(SOURCEDIR)\stonAivR.bmp 1 1 1

grd02056.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Blue/Gray Cracked Ivy Left" 1 $(SOURCEDIR)\stonBivL.bmp 1 1 1

grd02061.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Yellow Inca" -s 2 1 $(SOURCEDIR)\incatex.bmp 1 1 1

grd02074.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Red" -s 2 1 $(SOURCEDIR)\brwaltx1.bmp 1 1 1

grd02101.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Wall Normal (?)" 1 $(SOURCEDIR)\rckwaltx.bmp 1 1 1

grd02103.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Wall 2" 1 $(SOURCEDIR)\rok02tex.bmp 1 1 1

grd02104.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Wall 4" 1 $(SOURCEDIR)\rok04tex.bmp 1 1 1

grd02105.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Wall 2 Cave" -s 2 1 $(SOURCEDIR)\rokcav2.bmp 1 1 1

grd02121.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 1" 1 $(SOURCEDIR)\rokflor1.bmp 1 1 1

grd02122.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 2" 1 $(SOURCEDIR)\rokflor2.bmp 1 1 1

grd02123.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 3" 1 $(SOURCEDIR)\rokflr3p.bmp 1 1 1

grd02124.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 4" 1 $(SOURCEDIR)\rokflor4.bmp 1 1 1

grd02125.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 5" 1 $(SOURCEDIR)\rokflor5.bmp 1 1 1

grd02126.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 6" 1 $(SOURCEDIR)\rokflor6.bmp 1 1 1

grd02129.bgf:
	 makebgf -o $*.bgf -n "Rock Floor 9" 1 $(SOURCEDIR)\rokflor9.bmp 1 1 1

grd02201.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sand Wall" 1 $(SOURCEDIR)\sndwaltx.bmp 1 1 1

grd02301.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grass Green" 1 $(SOURCEDIR)\grastex2.bmp 1 1 1

grd02302.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grass Brown New 3/15/96" 1 $(SOURCEDIR)\brwngrs2.bmp 1 1 1

grd02303.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grass Brown 2" 1 $(SOURCEDIR)\bgras2tx.bmp 1 1 1

grd02304.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grass Brown 3" 1 $(SOURCEDIR)\bgras3tx.bmp 1 1 1

grd02305.bgf:
	 makebgf -o $*.bgf -n "Grass Brown 4" 1 $(SOURCEDIR)\bgras4tx.bmp 1 1 1

grd02306.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grass Brown 1 (mostly green)" 1 $(SOURCEDIR)\bgras1tx.bmp 1 1 1

grd02311.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grass Wall w/ Dirt" 1 $(SOURCEDIR)\graspath.bmp 1 1 1

grd02313.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Dirt Wall w/Grass" 1 $(SOURCEDIR)\dirtgras.bmp 1 1 1

grd02401.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Panel Wood 1" 1 $(SOURCEDIR)\wdpnltex.bmp 1 1 1

grd02402.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Panel Wood 2" 1 $(SOURCEDIR)\wdpnl2tx.bmp 1 1 1

grd02403.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Panel Wood Door" -s 2 1 $(SOURCEDIR)\wdpnldor.bmp 1 1 1

grd02404.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door Wood Dark" 1 $(SOURCEDIR)\doordk.bmp 1 1 1

grd02405.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door Wood Light" 1 $(SOURCEDIR)\doorlt.bmp 1 1 1

grd02501.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Alchemist" -s 2 1 $(SOURCEDIR)\alchsign.bmp 1 1 1

grd02502.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Inn" -s 2 1 $(SOURCEDIR)\innsign.bmp 1 1 1

grd02503.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Smith" -s 2 1 $(SOURCEDIR)\smthsign.bmp 1 1 1

grd02504.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Bank" -s 2 1 $(SOURCEDIR)\banksign.bmp 1 1 1

grd02505.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Toad" -s 2 1 $(SOURCEDIR)\toadsign.bmp 1 1 1

grd02506.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Alchemist 2" -s 1 1 $(SOURCEDIR)\alcmsign.bmp 1 1 1

grd02507.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Bed" -s 1 1 $(SOURCEDIR)\bedsign.bmp 1 1 1

grd02508.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Forestry" -s 1 1 $(SOURCEDIR)\frstsign.bmp 1 1 1

grd02509.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Hospital" -s 1 1 $(SOURCEDIR)\hospsign.bmp 1 1 1

grd02510.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Jewelry" -s 1 1 $(SOURCEDIR)\mchtsign.bmp 1 1 1

grd02511.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Cornucopia" -s 1 1 $(SOURCEDIR)\storsign.bmp 1 1 1

grd02512.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Tavern" -s 1 1 $(SOURCEDIR)\tavnsign.bmp 1 1 1

grd02513.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Hall/News ball" -s 1 1 $(SOURCEDIR)\wiswsign.bmp 1 1 1

grd02515.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Knight" -s 1 1 $(SOURCEDIR)\knhtsign.bmp 1 1 1

grd02701.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Archway stone space on top" -s 2 1 $(SOURCEDIR)\citarcex.bmp 1 1 1

grd02702.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Archway stone" -s 2 1 $(SOURCEDIR)\cityarch.bmp 1 1 1

grd02801.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Gateway stone tall " -s 2 1 $(SOURCEDIR)\citgatex.bmp 1 1 1

grd02802.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Gateway stone" -s 2 1 $(SOURCEDIR)\citygate.bmp 1 1 1

grd02803.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Gateway stone 2" -s 2 1 $(SOURCEDIR)\citygat2.bmp 1 1 1

grd02901.bgf:
	 makebgf -o $*.bgf -n "Inn Ceiling" 1 $(SOURCEDIR)\innceilg.bmp 1 1 1

grd02903.bgf:
	 makebgf -o $*.bgf -n "Inn Floor" 1 $(SOURCEDIR)\innfloor.bmp 1 1 1

grd03001.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall City/castle like" -s 1 1 $(SOURCEDIR)\wall.bmp 1 1 1

grd03002.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall City/castle like with slit" -s 1 1 $(SOURCEDIR)\wallslit.bmp 1 1 1

grd03003.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall transition to something" -s 1 1 $(SOURCEDIR)\transwll.bmp 1 1 1

grd03004.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall City/castle like Tall" -s 1 1 $(SOURCEDIR)\wallext.bmp 1 1 1

grd03005.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall City/castle like with slit Tall" -s 1 1 $(SOURCEDIR)\walslext.bmp 1 1 1

grd03006.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall transition to something Tall" -s 1 1 $(SOURCEDIR)\trnswlex.bmp 1 1 1

grd03101.bgf:
	 makebgf -o $*.bgf -n "Bar Top" -s 1 1 $(SOURCEDIR)\bartop.bmp 1 1 1

grd03102.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Bar Side" -s 1 1 $(SOURCEDIR)\barside.bmp 1 1 1

grd03201.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reddish Hell" -s 1 1 $(SOURCEDIR)\hellb.bmp 1 1 1

grd03203.bgf:
	 makebgf -o $*.bgf -n "Reddish Hell Floor" -s 1 1 $(SOURCEDIR)\hellflor.bmp 1 1 1

grd03204.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Reddish Hell lava glow" -s 1 2 $(SOURCEDIR)\hellA.bmp \
	 $(SOURCEDIR)\hellB.bmp 2 1 1 1 2

grd03301.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web grayish solid" -s 1 1 $(SOURCEDIR)\weberth.bmp 1 1 1

grd03302.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Web grayish transparent" -s 1 1 $(SOURCEDIR)\transweb.bmp 1 1 1

grd03401.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall" -s 1 1 $(SOURCEDIR)\mausolum.bmp 1 1 1

grd03402.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Door" -s 1 1 $(SOURCEDIR)\mausodor.bmp 1 1 1

grd03403.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Brick" -s 1 1 $(SOURCEDIR)\mausbrk.bmp 1 1 1

grd03404.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 2" -s 2 1 $(SOURCEDIR)\mauswal2.bmp 1 1 1

grd03405.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 3" -s 2 1 $(SOURCEDIR)\mauswal3.bmp 1 1 1

grd03407.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 5" -s 2 1 $(SOURCEDIR)\mauswal5.bmp 1 1 1

grd03408.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 6" -s 2 1 $(SOURCEDIR)\mauswal6.bmp 1 1 1

grd03409.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 7" -s 2 1 $(SOURCEDIR)\mauswal7.bmp 1 1 1

grd03410.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 1" -s 2 1 $(SOURCEDIR)\mauswal1.bmp 1 1 1

grd03411.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 8" -s 2 1 $(SOURCEDIR)\mauswal8.bmp 1 1 1

grd03412.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Wall 9" -s 2 1 $(SOURCEDIR)\mauswal9.bmp 1 1 1

grd03501.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Hermit Door" -s 1 1 $(SOURCEDIR)\hermdoor.bmp 1 1 1

grd03502.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Hermit Wall" -s 1 1 $(SOURCEDIR)\hermwall.bmp 1 1 1

grd03701.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Shelf Plain" -s 1 1 $(SOURCEDIR)\shelf2.bmp 1 1 1

grd03702.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Shelf Alchemist" -s 2 1 $(SOURCEDIR)\alcshlf2.bmp 1 1 1

grd03703.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Shelf Bar" -s 1 1 $(SOURCEDIR)\barshlf2.bmp 1 1 1

grd03704.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Shelf Bar shrunk 2" -s 2 1 $(SOURCEDIR)\barshlf2.bmp 1 1 1

grd03801.bgf:
	 makebgf -o $*.bgf -n "Cobblestone Brown" -s 1 1 $(SOURCEDIR)\cobble1.bmp 1 1 1

grd03802.bgf:
	 makebgf -o $*.bgf -n "Cobblestone Brown w/ Grass" -s 1 1 $(SOURCEDIR)\cobble2.bmp 1 1 1

grd03803.bgf:
	 makebgf -o $*.bgf -n "Cobblestone Brick" -s 1 1 $(SOURCEDIR)\cobbbrik.bmp 1 1 1

grd03811.bgf:
	 makebgf -o $*.bgf -n "Path Generic" -s 1 1 $(SOURCEDIR)\path.bmp 1 1 1

grd03901.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Window" -s 1 1 $(SOURCEDIR)\window.bmp 1 1 1

grd03902.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Window 2" -s 1 1 $(SOURCEDIR)\wintrans.bmp 1 1 1

grd03903.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Window 3" -s 1 1 $(SOURCEDIR)\wintran2.bmp 1 1 1

grd04001.bgf:
	 makebgf -o $*.bgf -n "Marble Tile 1" -s 1 1 $(SOURCEDIR)\martil1.bmp 1 1 1

grd04002.bgf:
	 makebgf -o $*.bgf -n "Marble Tile 1a" -s 1 1 $(SOURCEDIR)\martil1a.bmp 1 1 1

grd04003.bgf:
	 makebgf -o $*.bgf -n "Marble Tile 2" -s 1 1 $(SOURCEDIR)\martil2.bmp 1 1 1

grd04004.bgf:
	 makebgf -o $*.bgf -n "Marble Tile 2a" -s 1 1 $(SOURCEDIR)\martil2a.bmp 1 1 1

grd04005.bgf:
	 makebgf -o $*.bgf -n "Marble Tile 3" -s 1 1 $(SOURCEDIR)\martil3.bmp 1 1 1

grd04006.bgf:
	 makebgf -o $*.bgf -n "Marble Tile 3a" -s 1 1 $(SOURCEDIR)\martil3a.bmp 1 1 1

grd04101.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Stable Door" -s 1 1 $(SOURCEDIR)\stbldoor.bmp 1 1 1

grd04201.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Tile MRB " -s 1 1 $(SOURCEDIR)\mrbtile.bmp 1 1 1

grd04202.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Marble Tile MRB 1" -s 1 1 $(SOURCEDIR)\mrbtile1.bmp 1 1 1

grd04303.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Tile 3" -s 1 1 $(SOURCEDIR)\roktil03.bmp 1 1 1

grd04304.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Tile 4" -s 1 1 $(SOURCEDIR)\roktil04.bmp 1 1 1

grd04401.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 1" -s 1 1 $(SOURCEDIR)\waltil1m.bmp 1 1 1

grd04403.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 3" -s 1 1 $(SOURCEDIR)\waltil3m.bmp 1 1 1

grd04404.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 4" -s 1 1 $(SOURCEDIR)\waltil4m.bmp 1 1 1

grd04406.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 6" -s 1 1 $(SOURCEDIR)\waltil6m.bmp 1 1 1

grd04407.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 7" -s 1 1 $(SOURCEDIR)\waltil7m.bmp 1 1 1

grd04408.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 8" -s 1 1 $(SOURCEDIR)\waltil8m.bmp 1 1 1

grd04410.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile M 0" -s 1 1 $(SOURCEDIR)\waltil0m.bmp 1 1 1

grd04501.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 1" -s 1 1 $(SOURCEDIR)\waltil1r.bmp 1 1 1

grd04502.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 2" -s 1 1 $(SOURCEDIR)\waltil2r.bmp 1 1 1

grd04503.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 3" -s 1 1 $(SOURCEDIR)\waltil3r.bmp 1 1 1

grd04504.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 4" -s 1 1 $(SOURCEDIR)\waltil4r.bmp 1 1 1

grd04505.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 5" -s 1 1 $(SOURCEDIR)\waltil5r.bmp 1 1 1

grd04507.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 7" -s 1 1 $(SOURCEDIR)\waltil7r.bmp 1 1 1

grd04508.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile R 8" -s 1 1 $(SOURCEDIR)\waltil8r.bmp 1 1 1

grd04601.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 1" -s 1 1 $(SOURCEDIR)\waltil1w.bmp 1 1 1

grd04602.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 2" -s 1 1 $(SOURCEDIR)\waltil2w.bmp 1 1 1

grd04603.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 3" -s 1 1 $(SOURCEDIR)\waltil3w.bmp 1 1 1

grd04604.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 4" -s 1 1 $(SOURCEDIR)\waltil4w.bmp 1 1 1

grd04605.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 5" -s 1 1 $(SOURCEDIR)\waltil5w.bmp 1 1 1

grd04606.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 6" -s 1 1 $(SOURCEDIR)\waltil6w.bmp 1 1 1

grd04607.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 7" -s 1 1 $(SOURCEDIR)\waltil7w.bmp 1 1 1

grd04608.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 8" -s 1 1 $(SOURCEDIR)\waltil8w.bmp 1 1 1

grd04610.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile W 0" -s 1 1 $(SOURCEDIR)\waltil0w.bmp 1 1 1

grd04701.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile   1" -s 1 1 $(SOURCEDIR)\waltile1.bmp 1 1 1

grd04702.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile   2" -s 1 1 $(SOURCEDIR)\waltile2.bmp 1 1 1

grd04703.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile   3" -s 1 1 $(SOURCEDIR)\waltile3.bmp 1 1 1

grd04704.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile   4" -s 1 1 $(SOURCEDIR)\waltile4.bmp 1 1 1

grd04705.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile   5" -s 1 1 $(SOURCEDIR)\waltile5.bmp 1 1 1

grd04709.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile   9" -s 1 1 $(SOURCEDIR)\waltil9.bmp 1 1 1

grd04711.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile  11" -s 1 1 $(SOURCEDIR)\waltil11.bmp 1 1 1

grd04712.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile  12" -s 1 1 $(SOURCEDIR)\waltil12.bmp 1 1 1

grd04713.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile  13" -s 1 1 $(SOURCEDIR)\waltil13.bmp 1 1 1

grd04714.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Tile  14" -s 1 1 $(SOURCEDIR)\waltil14.bmp 1 1 1

grd04801.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Crown" -s 1 1 $(SOURCEDIR)\crowntap.bmp 1 1 1

grd04802.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Eagle" -s 1 1 $(SOURCEDIR)\eagletap.bmp 1 1 1

grd04803.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Fleur" -s 1 1 $(SOURCEDIR)\fleurtap.bmp 1 1 1

grd04804.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Flower" -s 1 1 $(SOURCEDIR)\flwrtap.bmp 1 1 1

grd04805.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Griffen" -s 1 1 $(SOURCEDIR)\grifntap.bmp 1 1 1

grd04806.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Horse" -s 1 1 $(SOURCEDIR)\hrsetap.bmp 1 1 1

grd04807.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Lion" -s 1 1 $(SOURCEDIR)\liontap.bmp 1 1 1

grd04808.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Sun" -s 1 1 $(SOURCEDIR)\suntap.bmp 1 1 1

grd04809.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Sword" -s 1 1 $(SOURCEDIR)\swordtap.bmp 1 1 1

grd04810.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Dragon with transparency" -s 2 1 $(SOURCEDIR)\tapestx3.bmp 1 1 1

grd04811.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Friends of the Forest" -s 1 1 $(SOURCEDIR)\woodstap.bmp 1 1 1

grd04812.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Tapestry Sun with transparency" -s 1 1 $(SOURCEDIR)\suntap2.bmp 1 1 1

grd04901.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Farm Floor" -s 2 1 $(SOURCEDIR)\farmwal2.bmp 1 1 1

grd05102.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wall Castle Cell 2" -s 2 1 $(SOURCEDIR)\cstlcel2.bmp 1 1 1

grd05201.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Arena Seat Back" -s 1 1 $(SOURCEDIR)\seatback.bmp 1 1 1

grd05404.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Ice 1" -s 1 1 $(SOURCEDIR)\icetex1.bmp 1 1 1

grd05405.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Ice 2" -s 1 1 $(SOURCEDIR)\icetex2.bmp 1 1 1

grd05417.bgf:
	makebgf -o $*.bgf $(WALL) -n "Ice blue2la.bmp" -s 1 1 $(SOURCEDIR)\blue2la.bmp 1 1 1

grd05418.bgf:
	makebgf -o $*.bgf $(WALL) -n "Ice blue2lb.bmp" -s 1 1 $(SOURCEDIR)\blue2lb.bmp 1 1 1

grd05419.bgf:
	makebgf -o $*.bgf $(WALL) -n "Ice blue2ra.bmp" -s 1 1 $(SOURCEDIR)\blue2ra.bmp 1 1 1

grd05420.bgf:
	makebgf -o $*.bgf $(WALL) -n "Ice blue2rb.bmp" -s 1 1 $(SOURCEDIR)\blue2rb.bmp 1 1 1

grd05441.bgf:
	makebgf -o $*.bgf $(WALL) -n "Ice icete2.bmp" -s 1 1 $(SOURCEDIR)\icete2.bmp 1 1 1

grd05504.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brownish 4" -s 1 1 $(SOURCEDIR)\briktex7.bmp 1 1 1

grd05507.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Brick Brownish 7" -s 1 1 $(SOURCEDIR)\brktex4a.bmp 1 1 1

grd05901.bgf:
	 makebgf -o grd05901.bgf -n "Carpet Red Top" -s 1 \
	1 $(SOURCEDIR)\rdcatex4.bmp 1 1 1

grd05902.bgf:
	makebgf -o grd05902.bgf -n "Carpet Red Bottom" -s 1 \
	1 $(SOURCEDIR)\rdcatex5.bmp 1 1 1

grd05903.bgf:
	makebgf -o grd05903.bgf -n "Carpet Red Corner Btm Lft" -s 1 \
	1 $(SOURCEDIR)\rdcatex6.bmp 1 1 1

grd05904.bgf:
	makebgf -o grd05904.bgf -n "Carpet Red Corner Top Lft" -s 1 \
	1 $(SOURCEDIR)\rdcatex7.bmp 1 1 1

grd05905.bgf:
	makebgf -o grd05905.bgf -n "Carpet Red Corner Top Rght" -s 1 \
	1 $(SOURCEDIR)\rdcatex8.bmp 1 1 1

grd05906.bgf:
	makebgf -o grd05906.bgf -n "Carpet Red Corner Btm Rght" -s 1 \
	1 $(SOURCEDIR)\rdcatex9.bmp 1 1 1

grd06001.bgf:
	 makebgf -o $*.bgf -n "Dirt 1" -s 1 1 $(SOURCEDIR)\dirt1.bmp 1 1 1

grd06002.bgf:
	 makebgf -o $*.bgf -n "Dirt 2" -s 1 1 $(SOURCEDIR)\dirt2.bmp 1 1 1

grd06004.bgf:
	 makebgf -o $*.bgf -n "Dirt 4" -s 1 1 $(SOURCEDIR)\dirt4.bmp 1 1 1

grd06501.bgf:
	makebgf -o $*.bgf $(WALL) -n "Clock Face" -s 2 12 \
	$(SOURCEDIR)\12oclock.bmp \
	$(SOURCEDIR)\1oclock.bmp \
	$(SOURCEDIR)\2oclock.bmp \
	$(SOURCEDIR)\3oclock.bmp \
	$(SOURCEDIR)\4oclock.bmp \
	$(SOURCEDIR)\5oclock.bmp \
	$(SOURCEDIR)\6oclock.bmp \
	$(SOURCEDIR)\7oclock.bmp \
	$(SOURCEDIR)\8oclock.bmp \
	$(SOURCEDIR)\9oclock.bmp \
	$(SOURCEDIR)\10oclock.bmp \
	$(SOURCEDIR)\11oclock.bmp \
	48 \
	1 1 1 2 1 3 1 4 1 5 1 6 1 7 1 8 1 9 1 10 1 11 1 12 \
	1 1 1 2 1 3 1 4 1 5 1 6 1 7 1 8 1 9 1 10 1 11 1 12 \
	1 12 1 11 1 10 1 9 1 8 1 7 1 6 1 5 1 4 1 3 1 2 1 1 \
	1 12 1 11 1 10 1 9 1 8 1 7 1 6 1 5 1 4 1 3 1 2 1 1 

grd06502.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Clock Brick" -s 2 1 $(SOURCEDIR)\briktex.bmp 1 1 1

grd06503.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Clock Brick Left" -s 2 1 $(SOURCEDIR)\briktexl.bmp 1 1 1

grd06601.bgf:
	 makebgf -o $*.bgf -n "Stone Floor Tile Tim" -s 1 1 $(SOURCEDIR)\stonchk1.bmp 1 1 1

grd06602.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Banister Metal 32 Tim" -s 2 1 $(SOURCEDIR)\banstr32.bmp 1 1 1

grd06603.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Banister Metal 48 Tim" -s 2 1 $(SOURCEDIR)\banstr48.bmp 1 1 1

grd06701.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door Plank" -s 1 1 $(SOURCEDIR)\plankdor.bmp 1 1 1

grd07000.bgf:
	makebgf -o $*.bgf $(WALL) -n "Shop keeper window" -s 2 1 $(SOURCEDIR)\bnkgrate.BMP 1 1 1        

grd07001.bgf:
	makebgf -o $*.bgf $(WALL) -n "Shelf Swords" -s 3 1 $(SOURCEDIR)\swrdshlf.BMP 1 1 1

grd07101.bgf:
	makebgf -o $*.bgf $(WALL) -n "Forest Jagged" 1 $(SOURCEDIR)\jagged1w.BMP 1 1 1

grd07201.bgf:
	makebgf -o $*.bgf -n "Water East" -s 2 5 \
	$(SOURCEDIR)\streamE1.bmp \
	$(SOURCEDIR)\streamE2.bmp \
	$(SOURCEDIR)\streamE3.bmp \
	$(SOURCEDIR)\streamE4.bmp \
	$(SOURCEDIR)\streamE5.bmp \
	5 1 1 1 2 1 3 1 4 1 5

grd07202.bgf:
	makebgf -o $*.bgf -n "Water West" -s 2 5 \
	$(SOURCEDIR)\streamW1.bmp \
	$(SOURCEDIR)\streamW2.bmp \
	$(SOURCEDIR)\streamW3.bmp \
	$(SOURCEDIR)\streamW4.bmp \
	$(SOURCEDIR)\streamW5.bmp \
	5 1 1 1 2 1 3 1 4 1 5

grd07203.bgf:
	makebgf -o $*.bgf -n "Water North" -s 2 5 \
	$(SOURCEDIR)\streamN1.bmp \
	$(SOURCEDIR)\streamN2.bmp \
	$(SOURCEDIR)\streamN3.bmp \
	$(SOURCEDIR)\streamN4.bmp \
	$(SOURCEDIR)\streamN5.bmp \
	5 1 1 1 2 1 3 1 4 1 5

grd07204.bgf:
	makebgf -o $*.bgf -n "Water South" -s 2 5 \
	$(SOURCEDIR)\streamS1.bmp \
	$(SOURCEDIR)\streamS2.bmp \
	$(SOURCEDIR)\streamS3.bmp \
	$(SOURCEDIR)\streamS4.bmp \
	$(SOURCEDIR)\streamS5.bmp \
	5 1 1 1 2 1 3 1 4 1 5

grd07301.bgf:
	makebgf -o $*.bgf $(WALL) -n "Wood 1" -s 2 1 $(SOURCEDIR)\wood1.BMP 1 1 1

grd07302.bgf:
	makebgf -o $*.bgf $(WALL) -n "Wood 2" -s 2 1 $(SOURCEDIR)\wood2.BMP 1 1 1

grd07401.bgf:
	makebgf -o $*.bgf $(WALL) -n "Gate Wall" -s 1 1 $(SOURCEDIR)\gatewall.BMP 1 1 1

grd07501.bgf:
	makebgf -o $*.bgf -n "Door Trap" -s 1 1 $(SOURCEDIR)\trapdoor.BMP 1 1 1

grd07601.bgf:
	makebgf -o $*.bgf $(WALL) -n "Shelf Books" -s 2 1 $(SOURCEDIR)\bookshlf.BMP 1 1 1

grd07701.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tablecloth Side" -s 1 1 $(SOURCEDIR)\tblcloth.BMP 1 1 1

grd07702.bgf:
	makebgf -o $*.bgf -n "Tablecloth Top" -s 1 1 $(SOURCEDIR)\clothtop.BMP 1 1 1

grd07801.bgf:
	makebgf -o $*.bgf -n "Wood Angle 1" -s 1 1 $(SOURCEDIR)\woodang1.BMP 1 1 1

grd07802.bgf:
	makebgf -o $*.bgf -n "Wood Angle 2" -s 1 1 $(SOURCEDIR)\woodang2.BMP 1 1 1

grd08001.bgf:
	makebgf -o $*.bgf $(WALL) -n "LTPUBD" -s 2 1 $(SOURCEDIR)\ltpubd.bmp 1 1 1

grd08002.bgf:
	makebgf -o $*.bgf $(WALL) -n "LTPUBF1" -s 2 1 $(SOURCEDIR)\ltpubf1.bmp 1 1 1

grd08003.bgf:
	makebgf -o $*.bgf $(WALL) -n "LTPUBF2" -s 2 1 $(SOURCEDIR)\ltpubf2.bmp 1 1 1

grd08004.bgf:
	makebgf -o $*.bgf $(WALL) -n "LTPUBF3" -s 2 1 $(SOURCEDIR)\ltpubf3.bmp 1 1 1

grd08005.bgf:
	makebgf -o $*.bgf $(WALL) -n "LTPUBF4" -s 2 1 $(SOURCEDIR)\ltpubf4.bmp 1 1 1

grd08006.bgf:
	makebgf -o $*.bgf $(WALL) -n "LTPUBF5" -s 2 1 $(SOURCEDIR)\ltpubf5.bmp 1 1 1

grd08101.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Duke Painting" -s 3 1 $(SOURCEDIR)\tapduke.bmp 1 1 1

grd08102.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Duke 1" -s 1 1 $(SOURCEDIR)\duketap1.bmp 1 1 1

grd08103.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Duke 2" -s 2 1 $(SOURCEDIR)\duketap2.bmp 1 1 1

grd08104.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Princess 3" -s 2 1 $(SOURCEDIR)\printap3.bmp 1 1 1

grd08105.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Princess 2" -s 2 1 $(SOURCEDIR)\printap4.bmp 1 1 1

grd08106.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Princess Painting" -s 2 1 $(SOURCEDIR)\tapprin.bmp 1 1 1

grd08107.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Duke 3" -s 2 1 $(SOURCEDIR)\duketap3.bmp 1 1 1

grd08108.bgf:
	makebgf -o $*.bgf $(WALL) -n "Tapestry Princess 1" -s 2 1 $(SOURCEDIR)\printap1.bmp 1 1 1

grd08401.bgf:
	 makebgf -o $*.bgf -n "Tapestry Purple Plain Floor" -s 2 1 $(SOURCEDIR)\prtapest.bmp 1 1 1

grd08501.bgf:
	 makebgf -o $*.bgf -n "Brick Yellow Inca Ceiling" -s 2 1 $(SOURCEDIR)\incatex2.bmp 1 1 1

grd08601.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Facade Adventurer's Club" -s 2 1 $(SOURCEDIR)\adclub.bmp 1 1 1

grd08816.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 1" -s 2 1 $(SOURCEDIR)\border02.bmp 1 1 1 

grd08817.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 2" -s 2 1 $(SOURCEDIR)\konimazz.bmp 1 1 1 

grd08818.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 3" -s 2 1 $(SOURCEDIR)\konimayy.bmp 1 1 1 

grd08819.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 4" -s 2 1 $(SOURCEDIR)\konima01.bmp 1 1 1

grd08820.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 5" -s 2 1 $(SOURCEDIR)\konima02.bmp 1 1 1

grd08821.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 6" -s 2 1 $(SOURCEDIR)\konima08.bmp 1 1 1

grd08822.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 7" -s 2 1 $(SOURCEDIR)\konima09.bmp 1 1 1

grd08823.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 8" -s 2 1 $(SOURCEDIR)\konima10.bmp 1 1 1

grd08824.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 9" -s 2 1 $(SOURCEDIR)\konima11.bmp 1 1 1

grd08825.bgf:
	makebgf -o $*.bgf $(WALL) -n "Konima Wall 10" -s 2 1 $(SOURCEDIR)\konima12.bmp 1 1 1

grd08840.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Tap/Sign Traditional" -s 2 1 $(SOURCEDIR)\arena01a.bmp 1 1 1

grd08841.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Tap/Sign Last Man" -s 2 1 $(SOURCEDIR)\arena02a.bmp 1 1 1

grd08842.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Tap/Sign Rules" -s 2 1 $(SOURCEDIR)\arena03a.bmp 1 1 1

grd08843.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Tap/Sign Guild Vs Guild" -s 2 1 $(SOURCEDIR)\arena04a.bmp 1 1 1

grd08844.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Tap/Sign Battle Royale" -s 2 1 $(SOURCEDIR)\arena05a.bmp 1 1 1

grd08845.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena cage bars" -s 2 1 $(SOURCEDIR)\argate01.bmp 1 1 1

grd08846.bgf:
	makebgf -o $*.bgf -n "Arena Floor" -s 2 1 $(SOURCEDIR)\arfloor.bmp 1 1 1

grd08847.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Maze Wall 32" -s 2 1 $(SOURCEDIR)\armaze01.bmp 1 1 1

grd08848.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Maze Wall 45" -s 2 1 $(SOURCEDIR)\armaze02.bmp 1 1 1

grd08849.bgf:
	makebgf -o $*.bgf $(WALL) -n "Arena Maze Wall 64" -s 2 1 $(SOURCEDIR)\armaze03.bmp 1 1 1

grd08850.bgf:
	makebgf -o $*.bgf -n "Arena Maze Floor" -s 2 1 $(SOURCEDIR)\armaze04.bmp 1 1 1

grd08851.bgf:
	makebgf -o $*.bgf $(WALL) -n "Duke's tablecloth 1" -s 2 1 $(SOURCEDIR)\table01.bmp 1 1 1

grd08852.bgf:
	makebgf -o $*.bgf $(WALL) -n "Duke's tablecloth 2" -s 2 1 $(SOURCEDIR)\table02.bmp 1 1 1

grd08853.bgf:
	makebgf -o $*.bgf $(WALL) -n "Duke's tablecloth 3" -s 2 1 $(SOURCEDIR)\table03.bmp 1 1 1

grd08854.bgf:
	makebgf -o $*.bgf -n "Duke's tablecloth 4" -s 2 1 $(SOURCEDIR)\table03.bmp 1 1 1

grd08855.bgf:
	makebgf -o $*.bgf $(WALL) -n "Duke's tablecloth 5" -s 2 1 $(SOURCEDIR)\table04.bmp 1 1 1

grd08856.bgf:
	makebgf -o $*.bgf $(WALL) -n "Duke's Decorations 1" -s 1 1 $(SOURCEDIR)\bunting1.bmp 1 1 1

grd08857.bgf:
	makebgf -o $*.bgf $(WALL) -n "Duke's Decorations 2" -s 1 1 $(SOURCEDIR)\bunting2.bmp 1 1 1

grd08859.bgf:
	 makebgf -o $*.bgf -n "Duke's Carpet Right" 1 $(SOURCEDIR)\purcarp1.bmp 1 1 1

grd08860.bgf:
	 makebgf -o $*.bgf -n "Duke's Carpet Middle" 1 $(SOURCEDIR)\purcarp3.bmp 1 1 1

grd08861.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Very Good" -s 2 3 \
		$(SOURCEDIR)\08861w2a.bmp \
		$(SOURCEDIR)\08861w2b.bmp \
		$(SOURCEDIR)\08861w2c.bmp \
		4 1 1 1 2 1 3 1 2

grd08862.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Very Good Overhang" -s 2 3 \
		$(SOURCEDIR)\08862w2a.bmp \
		$(SOURCEDIR)\08862w2b.bmp \
		$(SOURCEDIR)\08862w2c.bmp \
		4 1 1 1 2 1 3 1 2

grd08863.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Good" -s 2 3 \
		$(SOURCEDIR)\08863w2a.bmp \
		$(SOURCEDIR)\08863w2b.bmp \
		$(SOURCEDIR)\08863w2c.bmp \
		4 1 1 1 2 1 3 1 2

grd08864.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Good Overhang" -s 2 3 \
		$(SOURCEDIR)\08864w2a.bmp \
		$(SOURCEDIR)\08864w2b.bmp \
		$(SOURCEDIR)\08864w2c.bmp \
		4 1 1 1 2 1 3 1 2

grd08865.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Neutral" -s 2 1 $(SOURCEDIR)\08865w2.bmp 1 1 1

grd08866.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Neutral Overhang" -s 2 1 $(SOURCEDIR)\08866w2.bmp 1 1 1

grd08867.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Evil" -s 2 1 $(SOURCEDIR)\08867w2.bmp 1 1 1

grd08868.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Evil Overhang" -s 2 1 $(SOURCEDIR)\08868w2.bmp 1 1 1

grd08869.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Very Evil" -s 2 1 $(SOURCEDIR)\08869w2.bmp 1 1 1

grd08870.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Very Evil Overhang" -s 2 1 $(SOURCEDIR)\08870w2.bmp 1 1 1

grd08871.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Fey Forest Very Evil Eyes" -s 2 4 \
		$(SOURCEDIR)\08869w2.bmp \
		$(SOURCEDIR)\08869w2.bmp \
		$(SOURCEDIR)\08869w2.bmp \
		$(SOURCEDIR)\08869w2.bmp \
		21 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 4 1 1 1 2 1 3

grd08872.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin A" -s 2 1 $(SOURCEDIR)\08872w2.bmp 1 1 1

grd08873.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin A Left" -s 2 1 $(SOURCEDIR)\08873w2.bmp 1 1 1

grd08874.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin A Right" -s 2 1 $(SOURCEDIR)\08874w2.bmp 1 1 1

grd08875.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin B" -s 2 1 $(SOURCEDIR)\08875w2.bmp 1 1 1

grd08876.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin B Left" -s 2 1 $(SOURCEDIR)\08876w2.bmp 1 1 1

grd08877.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin B Right" -s 2 1 $(SOURCEDIR)\08877w2.bmp 1 1 1

grd08878.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Shingle" -s 2 1 $(SOURCEDIR)\08878f2.bmp 1 1 1

grd08879.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Konima Wall 11" -s 2 1 $(SOURCEDIR)\08879w2.bmp 1 1 1

grd08880.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Konima Window 1" -s 2 1 $(SOURCEDIR)\08880w2.bmp 1 1 1

grd08881.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin A Seam Left" -s 2 1 $(SOURCEDIR)\08881w2.bmp 1 1 1

grd08882.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin A Seam Right" -s 2 1 $(SOURCEDIR)\08882w2.bmp 1 1 1

grd08883.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin B Seam Left" -s 2 1 $(SOURCEDIR)\08883w2.bmp 1 1 1

grd08884.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Log Cabin B Seam Right" -s 2 1 $(SOURCEDIR)\08884w2.bmp 1 1 1

grd08885.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sandy Rock Wall" -s 1 1 $(SOURCEDIR)\08885w1.bmp 1 1 1

grd08886.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Torch Attaches to wall" -s 2 3 $(SOURCEDIR)\08886w2A.bmp $(SOURCEDIR)\08886w2B.bmp $(SOURCEDIR)\08886w2C.bmp 3 1 1 1 2 1 3

grd08887.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Torch Cross [The other part =)]" -s 2 3 $(SOURCEDIR)\08887w2A.bmp $(SOURCEDIR)\08887w2B.bmp $(SOURCEDIR)\08887w2C.bmp 3 1 1 1 2 1 3

grd08888.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Slate Rock Wall" -s 2 1 $(SOURCEDIR)\08888w1.bmp 1 1 1

grd08889.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door with Window" -s 2 1 $(SOURCEDIR)\08889w2.bmp 1 1 1

grd08890.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Door with Irons" -s 2 1 $(SOURCEDIR)\08890w2.bmp 1 1 1

grd08891.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wood Barn Wall" -s 2 1 $(SOURCEDIR)\08891w2.bmp 1 1 1

grd08892.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Crate (64x64 Wall and Floor)" -s 1 1 $(SOURCEDIR)\08892w1.bmp 1 1 1

grd08893.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Crate (32x32 Wall and Floor)" -s 2 1 $(SOURCEDIR)\08893w1.bmp 1 1 1

grd08894.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Waterfall Scroll" -s 2 1 $(SOURCEDIR)\08894f2.bmp 1 1 1

grd08895.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Water Pool Scroll" -s 2 1 $(SOURCEDIR)\08895f2.bmp 1 1 1

grd08896.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 1 Edge" -s 2 1 $(SOURCEDIR)\08896w2.bmp 1 1 1

grd08897.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 1 Tops" -s 2 1 $(SOURCEDIR)\08897w2.bmp 1 1 1

grd08898.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 1 Floor" -s 1 1 $(SOURCEDIR)\08898f2.bmp 1 1 1

grd08899.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Farm Wall" -s 2 1 $(SOURCEDIR)\08899w2.bmp 1 1 1

grd08900.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Rock Farm Door" -s 2 1 $(SOURCEDIR)\08900w2.bmp 1 1 1

grd08901.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 2 Edge" -s 2 1 $(SOURCEDIR)\08901w2.bmp 1 1 1

grd08902.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 2 Top" -s 2 1 $(SOURCEDIR)\08902w2.bmp 1 1 1

grd08903.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 2 Floor" -s 1 1 $(SOURCEDIR)\08903f2.bmp 1 1 1

grd08904.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 3 Edge" -s 2 1 $(SOURCEDIR)\08904w2.bmp 1 1 1

grd08905.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 3 Top" -s 2 1 $(SOURCEDIR)\08905w2.bmp 1 1 1

grd08906.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Field 3 Floor" -s 1 1 $(SOURCEDIR)\08906f2.bmp 1 1 1

grd08907.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Wierd stone wall stuff" -s 2 1 $(SOURCEDIR)\08907w2.bmp 1 1 1

grd08908.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Grey plaster wall" -s 2 1 $(SOURCEDIR)\08908w2.bmp 1 1 1

grd08909.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Konima Wall 12" -s 2 1 $(SOURCEDIR)\08909w2.bmp 1 1 1

grd08911.bgf:
	 makebgf -o $*.bgf -n "Water North - South Scroll" -s 2 1 $(SOURCEDIR)\08911f2.bmp 1 1 1

grd08912.bgf:
	 makebgf -o $*.bgf -n "Water East - West Scroll" -s 2 1 $(SOURCEDIR)\08912f2.bmp 1 1 1

grd08913.bgf:
	 makebgf -o $*.bgf -n "Water NorthWest - SouthEast Scroll" -s 2 1 $(SOURCEDIR)\08913f2.bmp 1 1 1

grd08914.bgf:
	 makebgf -o $*.bgf -n "Water NorthEast - SouthWest Scroll" -s 2 1 $(SOURCEDIR)\08914f2.bmp 1 1 1

grd08915.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 1 (Floor)" -s 2 1 $(SOURCEDIR)\08915f2.bmp 1 1 1

grd08916.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 1" -s 4 1 $(SOURCEDIR)\08916w4.bmp 1 1 1

grd08917.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Steps 1" -s 2 1 $(SOURCEDIR)\08917f2.bmp 1 1 1

grd08918.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge Tile 1" -s 4 1 $(SOURCEDIR)\08918w4.bmp 1 1 1

grd08919.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge Tile 2" -s 4 1 $(SOURCEDIR)\08919w4.bmp 1 1 1

grd08920.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge Tile 3" -s 4 1 $(SOURCEDIR)\08920w4.bmp 1 1 1

grd08921.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge Tile 1 (Floor)" -s 2 1 $(SOURCEDIR)\08921f2.bmp 1 1 1

grd08922.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge Tile 2 (Floor)" -s 2 1 $(SOURCEDIR)\08922f2.bmp 1 1 1

grd08923.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge Tile 3 (Floor)" -s 2 1 $(SOURCEDIR)\08923f2.bmp 1 1 1

grd08924.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Sidewalk 1" -s 2 1 $(SOURCEDIR)\08924f2.bmp 1 1 1

grd08925.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Sidewalk 1 Slopes" -s 2 1 $(SOURCEDIR)\08925f2.bmp 1 1 1

grd08926.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Door 1" -s 4 1 $(SOURCEDIR)\08926w4.bmp 1 1 1

grd08927.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Door 2" -s 4 1 $(SOURCEDIR)\08927w4.bmp 1 1 1

grd08928.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Door 3" -s 4 1 $(SOURCEDIR)\08928w4.bmp 1 1 1

grd08929.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 4 Left" -s 4 1 $(SOURCEDIR)\08929w4.bmp 1 1 1

grd08930.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 4 Center" -s 4 1 $(SOURCEDIR)\08930w4.bmp 1 1 1

grd08931.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 4 Right" -s 4 1 $(SOURCEDIR)\08931w4.bmp 1 1 1

grd08932.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Bar Details 1" -s 4 1 $(SOURCEDIR)\08932w4.bmp 1 1 1

grd08933.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Bar Details 2" -s 4 1 $(SOURCEDIR)\08933w4.bmp 1 1 1

grd08934.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Bar Details 3" -s 4 1 $(SOURCEDIR)\08934w4.bmp 1 1 1

grd08935.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Bar Details 4" -s 4 1 $(SOURCEDIR)\08935w4.bmp 1 1 1

grd08936.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Bar Details 5" -s 4 1 $(SOURCEDIR)\08936w4.bmp 1 1 1

grd08937.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement TEMP TEXTURE DON'T USE" -s 2 1 $(SOURCEDIR)\08937f2.bmp 1 1 1

grd08938.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Detail Avar 1 Painting" -s 4 1 $(SOURCEDIR)\08938w4.bmp 1 1 1

grd08939.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Detail Avar 1 Carving" -s 4 1 $(SOURCEDIR)\08939w4.bmp 1 1 1

grd08940.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 5 Left" -s 4 1 $(SOURCEDIR)\08940w4.bmp 1 1 1

grd08941.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 5 Center" -s 4 1 $(SOURCEDIR)\08941w4.bmp 1 1 1

grd08942.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 5 Right" -s 4 1 $(SOURCEDIR)\08942w4.bmp 1 1 1

grd08943.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Hall of Heroes Railing" -s 2 1 $(SOURCEDIR)\08943f2.bmp 1 1 1

grd08944.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Store Wall Left" -s 4 1 $(SOURCEDIR)\08944w4.bmp 1 1 1

grd08945.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Store Wall Right" -s 4 1 $(SOURCEDIR)\08945w4.bmp 1 1 1

grd08946.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Store Door Left" -s 4 1 $(SOURCEDIR)\08946w4.bmp 1 1 1

grd08947.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Store Door Right" -s 4 1 $(SOURCEDIR)\08947w4.bmp 1 1 1

grd08948.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 5 Small Center" -s 4 1 $(SOURCEDIR)\08948w4.bmp 1 1 1

grd08949.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 5 Small Right" -s 4 1 $(SOURCEDIR)\08949w4.bmp 1 1 1

grd08950.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Ledge 5 Small Left" -s 4 1 $(SOURCEDIR)\08950w4.bmp 1 1 1

grd08951.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Hall of Heroes Left" -s 4 1 $(SOURCEDIR)\08951w4.bmp 1 1 1

grd08952.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Settlement Hall of Heroes Right " -s 4 1 $(SOURCEDIR)\08952w4.bmp 1 1 1

grd08953.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Cabinet 1" -s 2 1 $(SOURCEDIR)\08953w2.bmp 1 1 1

grd08954.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Cabinet 2" -s 2 1 $(SOURCEDIR)\08954w2.bmp 1 1 1

grd08955.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Cabinet 3" -s 2 1 $(SOURCEDIR)\08955w2.bmp 1 1 1

grd08956.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Cabinet 4" -s 2 1 $(SOURCEDIR)\08956w2.bmp 1 1 1

grd08957.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 1" -s 4 1 $(SOURCEDIR)\08957w4.bmp 1 1 1

grd08958.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 2" -s 4 1 $(SOURCEDIR)\08958w4.bmp 1 1 1

grd08959.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 3" -s 4 1 $(SOURCEDIR)\08959w4.bmp 1 1 1

grd08960.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 4" -s 4 1 $(SOURCEDIR)\08960w4.bmp 1 1 1

grd08961.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Floor 1" -s 2 1 $(SOURCEDIR)\08961f2.bmp 1 1 1

grd08962.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Post 1" -s 2 1 $(SOURCEDIR)\08962w2.bmp 1 1 1

grd08963.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Floor 2" -s 2 1 $(SOURCEDIR)\08963f2.bmp 1 1 1

grd08964.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Floor 3" -s 2 1 $(SOURCEDIR)\08964f2.bmp 1 1 1

grd08965.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Floor 4" -s 2 1 $(SOURCEDIR)\08965f2.bmp 1 1 1

grd08966.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Cabinet 5" -s 2 1 $(SOURCEDIR)\08966w2.bmp 1 1 1

grd08967.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 5" -s 2 1 $(SOURCEDIR)\08967w2.bmp 1 1 1

grd08968.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Floor 5" -s 2 1 $(SOURCEDIR)\08968f2.bmp 1 1 1

grd08969.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 6" -s 2 1 $(SOURCEDIR)\08969w2.bmp 1 1 1

grd08970.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 7" -s 4 1 $(SOURCEDIR)\08970w4.bmp 1 1 1

grd08971.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 8" -s 4 1 $(SOURCEDIR)\08971w4.bmp 1 1 1

grd08972.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Wall 9" -s 2 1 $(SOURCEDIR)\08972f2.bmp 1 1 1

grd08973.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Door 1" -s 4 1 $(SOURCEDIR)\08973w4.bmp 1 1 1

grd08974.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set outdoor floor" -s 2 1 $(SOURCEDIR)\08974f2.bmp 1 1 1

grd08975.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set light floor stair wall" -s 2 1 $(SOURCEDIR)\08975w2.bmp 1 1 1

grd08976.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set dark floor stair wall" -s 4 1 $(SOURCEDIR)\08976w4.bmp 1 1 1

grd08977.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set tablecloth top" -s 2 1 $(SOURCEDIR)\08977f2.bmp 1 1 1

grd08978.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar wall" -s 4 1 $(SOURCEDIR)\08978w4.bmp 1 1 1

grd08979.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Door Hall of Heroes" -s 4 1 $(SOURCEDIR)\08979w4.bmp 1 1 1

grd08980.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Guard Towers Door" -s 4 1 $(SOURCEDIR)\08980w4.bmp 1 1 1

grd08981.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Guard Towers Border" -s 2 1 $(SOURCEDIR)\08981w2.bmp 1 1 1

grd08982.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Barracks Door" -s 4 1 $(SOURCEDIR)\08982w4.bmp 1 1 1

grd08983.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Column Face" -s 4 1 $(SOURCEDIR)\08983w4.bmp 1 1 1

grd08984.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Column Side" -s 4 1 $(SOURCEDIR)\08984w4.bmp 1 1 1

grd08985.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Doorframe Left" -s 4 1 $(SOURCEDIR)\08985w4.bmp 1 1 1

grd08986.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Doorframe right" -s 4 1 $(SOURCEDIR)\08986w4.bmp 1 1 1

grd08987.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Wall Detail" -s 4 1 $(SOURCEDIR)\08987w4.bmp 1 1 1

grd08988.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Tilewall 1" -s 4 1 $(SOURCEDIR)\08988w4.bmp 1 1 1

grd08989.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Tilewall 2" -s 4 1 $(SOURCEDIR)\08989w4.bmp 1 1 1

grd08990.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Tilewall 3" -s 4 1 $(SOURCEDIR)\08990w4.bmp 1 1 1

grd08991.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW temp floor 1" -s 2 1 $(SOURCEDIR)\08991f2.bmp 1 1 1

grd08992.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW temp ceiling 1" -s 2 1 $(SOURCEDIR)\08992f2.bmp 1 1 1

grd08993.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW temp wall 1" -s 4 1 $(SOURCEDIR)\08993w4.bmp 1 1 1

grd08994.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Wall 2 (Living Areas)" -s 4 1 $(SOURCEDIR)\08994w4.bmp 1 1 1

grd08995.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Wall 3" -s 4 1 $(SOURCEDIR)\08995w4.bmp 1 1 1

grd08996.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Wall 4 (Living Areas)" -s 4 1 $(SOURCEDIR)\08996w4.bmp 1 1 1

grd08997.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Wall 5 (Living Areas)" -s 4 1 $(SOURCEDIR)\08997w4.bmp 1 1 1

grd08998.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Floor for Wall 5" -s 2 1 $(SOURCEDIR)\08998f2.bmp 1 1 1

grd08999.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Floor for Wall 2" -s 2 1 $(SOURCEDIR)\08999f2.bmp 1 1 1

grd09000.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Floor for Wall 3" -s 2 1 $(SOURCEDIR)\09000f2.bmp 1 1 1

grd09001.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Floor for Wall 4" -s 2 1 $(SOURCEDIR)\09001f2.bmp 1 1 1

grd09002.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Iron Gate" -s 4 1 $(SOURCEDIR)\09002w4.bmp 1 1 1

grd09003.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Iron Gate Up and Down" -s 4 1 $(SOURCEDIR)\09003w4.bmp 1 1 1

grd09004.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Iron Gate Metal Only" -s 4 1 $(SOURCEDIR)\09004w4.bmp 1 1 1

grd09005.bgf:
	makebgf -o $*.bgf -r -n "OW Sludge Scroll" -s 2 1 $(SOURCEDIR)\09005f2.BMP 1 1 1

grd09006.bgf:
	makebgf -o $*.bgf -r -n "OW Wall 2 W/ Waterline" -s 4 1 $(SOURCEDIR)\09006w4.BMP 1 1 1

grd09007.bgf:
	makebgf -o $*.bgf -r -n "OW Wall 3 W/ Waterline" -s 4 1 $(SOURCEDIR)\09007w4.BMP 1 1 1

grd09008.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Rock Carving 1" -s 4 1 $(SOURCEDIR)\09008.bmp 1 1 1

grd09009.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Rock Carving 2" -s 4 1 $(SOURCEDIR)\09009.bmp 1 1 1

grd09010.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Rock Carving 3" -s 4 1 $(SOURCEDIR)\09010.bmp 1 1 1

grd09011.bgf:
	 makebgf -o $*.bgf $(WALL) -n "OW Rock Carving 4" -s 4 1 $(SOURCEDIR)\09011.bmp 1 1 1

grd09012.bgf:
	makebgf -o $*.bgf -r -n "OW Rock Carving 5" -s 4 1 $(SOURCEDIR)\09012w4.BMP 1 1 1

grd09013.bgf:
	makebgf -o $*.bgf -r -n "OW Rock Carving 6" -s 4 1 $(SOURCEDIR)\09013w4.BMP 1 1 1

grd09014.bgf:
	makebgf -o $*.bgf -r -n "OW Puzzle Room Floor" -s 2 1 $(SOURCEDIR)\09014f2.BMP 1 1 1

grd09015.bgf:
	makebgf -o $*.bgf -r -n "OW Puzzle Room Tile 1A" -s 2 1 $(SOURCEDIR)\09015f2.BMP 1 1 1

grd09016.bgf:
	makebgf -o $*.bgf -r -n "OW Puzzle Room Tile 1B" -s 2 1 $(SOURCEDIR)\09016f2.BMP 1 1 1

grd09017.bgf:
	makebgf -o $*.bgf -r -n "OW Puzzle Room Tile 2A" -s 2 1 $(SOURCEDIR)\09017f2.BMP 1 1 1

grd09018.bgf:
	makebgf -o $*.bgf -r -n "OW Puzzle Room Tile 2B" -s 2 1 $(SOURCEDIR)\09018f2.BMP 1 1 1

grd09019.bgf:
	makebgf -o $*.bgf -r -n "OW Puzzle Room Chain" -s 1 1 $(SOURCEDIR)\09019w2.BMP 1 1 1

grd09020.bgf:
	makebgf -o $*.bgf -r -n "OW Wall 6" -s 4 1 $(SOURCEDIR)\09020w4.BMP 1 1 1

grd09021.bgf:
	makebgf -o $*.bgf -r -n "OW Wall 7" -s 4 1 $(SOURCEDIR)\09021w4.BMP 1 1 1

grd09030.bgf:
	makebgf -o $*.bgf -r -n "Set YF Tower Wall" -s 4 1 $(SOURCEDIR)\09030w4.BMP 1 1 1

grd09031.bgf:
	makebgf -o $*.bgf -r -n "Set YF Towerwall Wall" -s 4 1 $(SOURCEDIR)\09031w4.BMP 1 1 1

grd09032.bgf:
	makebgf -o $*.bgf -r -n "Set YF Wall 1" -s 4 1 $(SOURCEDIR)\09032w4.BMP 1 1 1

grd09033.bgf:
	makebgf -o $*.bgf -r -n "Set YF Wall 2" -s 4 1 $(SOURCEDIR)\09033w4.BMP 1 1 1

grd09034.bgf:
	makebgf -o $*.bgf -r -n "Set YF Floor 1" -s 2 1 $(SOURCEDIR)\09034f2.BMP 1 1 1

grd09035.bgf:
	makebgf -o $*.bgf -r -n "Set YF Floor 2" -s 2 1 $(SOURCEDIR)\09035f2.BMP 1 1 1

grd09036.bgf:
	makebgf -o $*.bgf -r -n "Set YF Floor 3" -s 2 1 $(SOURCEDIR)\09036f2.BMP 1 1 1

grd09037.bgf:
	makebgf -o $*.bgf -r -n "Set YF Far Tower" -s 4 1 $(SOURCEDIR)\09037w4.BMP 1 1 1

grd09066.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle W/ 2 Trees" -s 2 1 $(SOURCEDIR)\09066w4.bmp 1 1 1

grd09067.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Canopy 1" -s 2 1 $(SOURCEDIR)\09067f2.bmp 1 1 1

grd09068.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Canopy 2" -s 2 1 $(SOURCEDIR)\09068f2.bmp 1 1 1

grd09069.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Floor 1" -s 2 1 $(SOURCEDIR)\09069f2.bmp 1 1 1

grd09070.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Bushes W/ Trans 1" -s 4 1 $(SOURCEDIR)\09070w4.bmp 1 1 1

grd09071.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Bushes W/ Trans 2" -s 2 1 $(SOURCEDIR)\09071w.bmp 1 1 1

grd09072.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Bark 1 for 3D Tree" -s 2 1 $(SOURCEDIR)\09072.bmp 1 1 1

grd09073.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Bark 2 for 3D Tree" -s 2 1 $(SOURCEDIR)\09073.bmp 1 1 1

grd09074.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Roots 1 for 3D Tree" -s 2 1 $(SOURCEDIR)\09074.bmp 1 1 1

grd09075.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Sand 1" 1 $(SOURCEDIR)\09075f1.bmp 1 1 1

grd09076.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Sand 2" 1 $(SOURCEDIR)\09076f1.bmp 1 1 1

grd09077.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Sand 3 Windblown Dunes" 1 $(SOURCEDIR)\09077f1.bmp 1 1 1

grd09078.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Cliff 1" -s 2 1 $(SOURCEDIR)\09078f2.bmp 1 1 1

grd09079.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Cliff 2" -s 1 1 $(SOURCEDIR)\09095w4.bmp 1 1 1

grd09080.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Rocky Ground Dark (For Cliff 09079)" -s 2 1 $(SOURCEDIR)\09080f2.bmp 1 1 1

grd09081.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Rocky Ground Light (For Cliff 09078)" -s 2 1 $(SOURCEDIR)\09081f2.bmp 1 1 1

grd09082.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Dirtpath" -s 2 1 $(SOURCEDIR)\09082f2.bmp 1 1 1

grd09083.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Ocean 1 Far Away" -s 2 3 \
		$(SOURCEDIR)\09083f2a.bmp \
		$(SOURCEDIR)\09083f2b.bmp \
		$(SOURCEDIR)\09083f2c.bmp \
		3 1 1 1 2 1 3

grd09084.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Ocean 2 Closer" -s 2 3 \
		$(SOURCEDIR)\09084f2a.bmp \
		$(SOURCEDIR)\09084f2b.bmp \
		$(SOURCEDIR)\09084f2c.bmp \
		3 1 1 1 2 1 3

grd09085.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Waves On Sand 1" -s 2 3 \
		$(SOURCEDIR)\09085f2a.bmp \
		$(SOURCEDIR)\09085f2b.bmp \
		$(SOURCEDIR)\09085f2c.bmp \
		3 1 1 1 2 1 3

grd09086.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Waves On Sand 2" -s 2 3 \
		$(SOURCEDIR)\09086f2a.bmp \
		$(SOURCEDIR)\09086f2b.bmp \
		$(SOURCEDIR)\09086f2c.bmp \
		3 1 1 1 2 1 3

grd09087.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Waves On Sand 3" -s 2 3 \
		$(SOURCEDIR)\09087f2a.bmp \
		$(SOURCEDIR)\09087f2b.bmp \
		$(SOURCEDIR)\09087f2c.bmp \
		3 1 1 1 2 1 3

grd09088.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Island: Sand for Shore" -s 2 1 $(SOURCEDIR)\09088f2.bmp 1 1 1

grd09089.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle W/O Trees" -s 2 1 $(SOURCEDIR)\09089w4.bmp 1 1 1

grd09090.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle W/ 3 Trees" -s 2 1 $(SOURCEDIR)\09090w4.bmp 1 1 1

grd09091.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle W/ 1 Tree" -s 2 1 $(SOURCEDIR)\09091w4.bmp 1 1 1

grd09092.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle W/O Trees W/ Transparency" -s 2 1 $(SOURCEDIR)\09092w4.bmp 1 1 1

grd09093.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Bark 3 for 3D Tree" -s 2 1 $(SOURCEDIR)\09093f2.bmp 1 1 1

grd09094.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Bark 3 W/ Grass for 3D Tree" -s 2 1 $(SOURCEDIR)\09094f2.bmp 1 1 1

grd09095.bgf:
	makebgf -o $*.bgf -r -n "Island: Cliff 3 (Huge)" -s 2 1 $(SOURCEDIR)\09095w4.BMP 1 1 1

grd09096.bgf:
	makebgf -o $*.bgf -r -n "Island: Jungle Huge for Outside Walls" -s 1 1 $(SOURCEDIR)\09096w4.BMP 1 1 1

grd09097.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Hive Wall 1 (Floor)" -s 2 1 $(SOURCEDIR)\09097w2.bmp 1 1 1

grd09098.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Hive Wall 2 (Floor)" -s 2 1 $(SOURCEDIR)\09098w2.bmp 1 1 1

grd09099.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Dragonfly Nest (Floor)" -s 2 1 $(SOURCEDIR)\09099w2.bmp 1 1 1

grd09100.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Haning Stuff" -s 2 1 $(SOURCEDIR)\09100w4.bmp 1 1 1

grd09104.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Hive Floor" -s 2 1 $(SOURCEDIR)\09104f2.bmp 1 1 1

grd09105.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Hive Wall 1" -s 4 1 $(SOURCEDIR)\09105w4.bmp 1 1 1

grd09106.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Hive Wall 2" -s 4 1 $(SOURCEDIR)\09106w4.bmp 1 1 1

grd09107.bgf:
	 makebgf -o $*.bgf $(WALL) -n "KA1 Dragonfly Nest" -s 4 1 $(SOURCEDIR)\09107w4.bmp 1 1 1

grd09108.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Transition Dark Rock to Jungle Floor" -s 2 1 $(SOURCEDIR)\09108f2.bmp 1 1 1

grd09109.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Jungle Transition Light Rock to Jungle Floor" -s 2 1 $(SOURCEDIR)\09109f2.bmp 1 1 1

grd09115.bgf:
	makebgf -o $*.bgf -r -n "KA3: Wall 1" -s 4 1 $(SOURCEDIR)\09115w4.BMP 1 1 1

grd09116.bgf:
	makebgf -o $*.bgf -r -n "KA3: Wall 2" -s 4 1 $(SOURCEDIR)\09116w4.BMP 1 1 1

grd09117.bgf:
	makebgf -o $*.bgf -r -n "KA3: Fireplace 1" -s 4 3 $(SOURCEDIR)\09117w4A.bmp $(SOURCEDIR)\09117w4B.bmp $(SOURCEDIR)\09117w4C.bmp 3 1 1 1 2 1 3

grd09118.bgf:
	makebgf -o $*.bgf -r -n "KA3: Floor 1" -s 2 1 $(SOURCEDIR)\09118f2.BMP 1 1 1

grd09119.bgf:
	makebgf -o $*.bgf -r -n "KA3: Roof 1" -s 2 1 $(SOURCEDIR)\09119f2.BMP 1 1 1

grd09120.bgf:
	makebgf -o $*.bgf -r -n "KA3: Worktable 1" -s 4 1 $(SOURCEDIR)\09120w4.BMP 1 1 1

grd09121.bgf:
	makebgf -o $*.bgf -r -n "KA3: Worktable 2" -s 4 1 $(SOURCEDIR)\09121w4.BMP 1 1 1

grd09122.bgf:
	makebgf -o $*.bgf -r -n "KA3: Table Top 1 (Arrowheads & Feathers)" -s 2 1 $(SOURCEDIR)\09122f2.BMP 1 1 1

grd09123.bgf:
	makebgf -o $*.bgf -r -n "KA3: Table Top 2 (Quiver)" -s 2 1 $(SOURCEDIR)\09123f2.BMP 1 1 1

grd09124.bgf:
	makebgf -o $*.bgf -r -n "KA3: Table Top 3 (Bow)" -s 2 1 $(SOURCEDIR)\09124f2.BMP 1 1 1

grd09125.bgf:
	makebgf -o $*.bgf -r -n "KA3: Table Top 4 (Plain Wood)" -s 2 1 $(SOURCEDIR)\09125f2.BMP 1 1 1

grd09126.bgf:
	makebgf -o $*.bgf -r -n "KA3: Door" -s 4 1 $(SOURCEDIR)\09126w4.BMP 1 1 1

grd09127.bgf:
	makebgf -o $*.bgf -r -n "KA3: Thatch Roof" -s 2 1 $(SOURCEDIR)\09127f2.BMP 1 1 1

grd09128.bgf:
	makebgf -o $*.bgf -r -n "KA3: Outside Wall" -s 4 1 $(SOURCEDIR)\09128w4.BMP 1 1 1

grd09129.bgf:
	makebgf -o $*.bgf -r -n "KA3: Door 2" -s 4 1 $(SOURCEDIR)\09129w4.BMP 1 1 1

grd09130.bgf:
	makebgf -o $*.bgf -r -n "KA3: Wall 3" -s 4 1 $(SOURCEDIR)\09130w4.BMP 1 1 1

grd09131.bgf:
	makebgf -o $*.bgf -r -n "KA3: Stone Chimney" -s 4 1 $(SOURCEDIR)\09131w4.BMP 1 1 1

grd09132.bgf:
	makebgf -o $*.bgf -r -n "KA3: Floor 2" -s 2 1 $(SOURCEDIR)\09132f2.BMP 1 1 1

grd09133.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 1" -s 2 1 $(SOURCEDIR)\09133w2.BMP 1 1 1

grd09134.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 2" -s 2 1 $(SOURCEDIR)\09134w2.BMP 1 1 1

grd09135.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 3" -s 2 1 $(SOURCEDIR)\09135w2.BMP 1 1 1

grd09136.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 4" -s 2 1 $(SOURCEDIR)\09136w2.BMP 1 1 1

grd09137.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 5" -s 2 1 $(SOURCEDIR)\09137w2.BMP 1 1 1

grd09138.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 6" -s 2 1 $(SOURCEDIR)\09138w2.BMP 1 1 1

grd09139.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 7" -s 2 1 $(SOURCEDIR)\09139w2.BMP 1 1 1

grd09140.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 8" -s 2 1 $(SOURCEDIR)\09140w2.BMP 1 1 1

grd09141.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 9" -s 2 1 $(SOURCEDIR)\09141w2.BMP 1 1 1

grd09142.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 10" -s 2 1 $(SOURCEDIR)\09142w2.BMP 1 1 1

grd09143.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 11" -s 2 1 $(SOURCEDIR)\09143w2.BMP 1 1 1

grd09144.bgf:
	makebgf -o $*.bgf -r -n "KA1 NewTex 12" -s 2 1 $(SOURCEDIR)\09144w2.BMP 1 1 1

grd09149.bgf:
	makebgf -o $*.bgf -r -n "KB2: Earthen Fort (Packed Dirt)" -s 2 1 $(SOURCEDIR)\09149f2.BMP 1 1 1

grd09151.bgf:
	makebgf -o $*.bgf -r -n "KC1: Chasm Bridge Side 1" -s 2 1 $(SOURCEDIR)\09151f2.BMP 1 1 1

grd09152.bgf:
	makebgf -o $*.bgf -r -n "KC1: Chasm Bridge Floor 1" -s 2 1 $(SOURCEDIR)\09152f2.BMP 1 1 1

grd09153.bgf:
	makebgf -o $*.bgf -r -n "KC1: Rope 1" -s 2 1 $(SOURCEDIR)\09153w2.BMP 1 1 1

grd09154.bgf:
	makebgf -o $*.bgf -r -n "KC1: Rope Post" -s 2 1 $(SOURCEDIR)\09154w2.BMP 1 1 1

grd09155.bgf:
	makebgf -o $*.bgf -r -n "KC1: Rope 2" -s 2 1 $(SOURCEDIR)\09155w2.BMP 1 1 1

grd09164.bgf:
	makebgf -o $*.bgf -r -n "KC3: Front Door Outside" -s 4 1 $(SOURCEDIR)\09164w4.BMP 1 1 1

grd09165.bgf:
	makebgf -o $*.bgf -r -n "KC3: Panel Sungod Large" -s 4 1 $(SOURCEDIR)\09165w4.BMP 1 1 1

grd09166.bgf:
	makebgf -o $*.bgf -r -n "KC3: Panel 1 Outside" -s 4 1 $(SOURCEDIR)\09166w4.BMP 1 1 1

grd09167.bgf:
	makebgf -o $*.bgf -r -n "KC3: Panel 2 Outside" -s 4 1 $(SOURCEDIR)\09167w4.BMP 1 1 1

grd09168.bgf:
	makebgf -o $*.bgf -r -n "KC3: Panel 3 Outside" -s 4 1 $(SOURCEDIR)\09168w4.BMP 1 1 1

grd09169.bgf:
	makebgf -o $*.bgf -r -n "KC3: Panel 4 Outside" -s 4 1 $(SOURCEDIR)\09169w4.BMP 1 1 1

grd09170.bgf:
	makebgf -o $*.bgf -r -n "KC3: Panel 5 Outside" -s 4 1 $(SOURCEDIR)\09170w4.BMP 1 1 1

grd09171.bgf:
	makebgf -o $*.bgf -r -n "KC3: Obelisk Panel 1" -s 4 1 $(SOURCEDIR)\09171w4.BMP 1 1 1

grd09172.bgf:
	makebgf -o $*.bgf -r -n "KC3: Obelisk Panel 2" -s 4 1 $(SOURCEDIR)\09172w4.BMP 1 1 1

grd09173.bgf:
	makebgf -o $*.bgf -r -n "KC3: Obelisk Panel 3" -s 4 1 $(SOURCEDIR)\09173w4.BMP 1 1 1

grd09174.bgf:
	makebgf -o $*.bgf -r -n "KC3: Obelisk Panel 4" -s 4 1 $(SOURCEDIR)\09174w4.BMP 1 1 1

grd09175.bgf:
	makebgf -o $*.bgf -r -n "KC3: Floor 1 (Inside)" -s 2 1 $(SOURCEDIR)\09175f2.BMP 1 1 1

grd09176.bgf:
	makebgf -o $*.bgf -r -n "KC3: Inside Bricks" -s 2 1 $(SOURCEDIR)\09176f2.BMP 1 1 1

grd09177.bgf:
	makebgf -o $*.bgf -r -n "KC3: Ceiling 1" -s 2 1 $(SOURCEDIR)\09177f2.BMP 1 1 1

grd09178.bgf:
	makebgf -o $*.bgf -r -n "KC3: Stone Path to Temple" -s 2 1 $(SOURCEDIR)\09178f2.BMP 1 1 1

grd09190.bgf:
	makebgf -o $*.bgf -r -n "KC5: Wall 1" -s 4 1 $(SOURCEDIR)\09190w4.BMP 1 1 1

grd09191.bgf:
	makebgf -o $*.bgf -r -n "KC5: Wall 2" -s 4 1 $(SOURCEDIR)\09191w4.BMP 1 1 1

grd09192.bgf:
	makebgf -o $*.bgf -r -n "KC5: Wall 3" -s 4 1 $(SOURCEDIR)\09192w4.BMP 1 1 1

grd09193.bgf:
	makebgf -o $*.bgf -r -n "KC5: Wall 4" -s 4 1 $(SOURCEDIR)\09193w4.BMP 1 1 1

grd09194.bgf:
	makebgf -o $*.bgf -r -n "KC5: Fireplace Wall (Animated)" -s 4 3 $(SOURCEDIR)\09194w4A.bmp $(SOURCEDIR)\09194w4B.bmp $(SOURCEDIR)\09194w4C.bmp 3 1 1 1 2 1 3

grd09195.bgf:
	makebgf -o $*.bgf -r -n "KC5: Door 1" -s 4 1 $(SOURCEDIR)\09195w4.BMP 1 1 1

grd09196.bgf:
	makebgf -o $*.bgf -r -n "KC5: Floor 1" -s 2 1 $(SOURCEDIR)\09196f2.BMP 1 1 1

grd09197.bgf:
	makebgf -o $*.bgf -r -n "KC5: Ceiling 1" -s 2 1 $(SOURCEDIR)\09197f2.BMP 1 1 1

grd09198.bgf:
	makebgf -o $*.bgf -r -n "KC5: Wall 5 (Outside)" -s 2 1 $(SOURCEDIR)\09198w2.BMP 1 1 1

grd09199.bgf:
	makebgf -o $*.bgf -r -n "KC5: Fence 1 (Rustic Stick)" -s 4 1 $(SOURCEDIR)\09199w4.BMP 1 1 1

grd09200.bgf:
	makebgf -o $*.bgf -r -n "KC5: Roof 1 (Thatch)" -s 2 1 $(SOURCEDIR)\09200f2.BMP 1 1 1

grd09201.bgf:
	makebgf -o $*.bgf -n "KC5: Table Top 1" -s 2 1 $(SOURCEDIR)\09201f2.BMP 1 1 1

grd09202.bgf:
	makebgf -o $*.bgf -r -n "KC5: Table Side 1" -s 2 1 $(SOURCEDIR)\09202w2.BMP 1 1 1

grd09211.bgf:
	makebgf -o $*.bgf -r -n "KD1: Wall 1" -s 4 1 $(SOURCEDIR)\09211w4.BMP 1 1 1

grd09212.bgf:
	makebgf -o $*.bgf -r -n "KD1: Wall 2 (Skull Totem)" -s 4 1 $(SOURCEDIR)\09212w4.BMP 1 1 1

grd09213.bgf:
	makebgf -o $*.bgf -r -n "KD1: Wall 3 (Mess of Bones)" -s 4 1 $(SOURCEDIR)\09213w4.BMP 1 1 1

grd09214.bgf:
	makebgf -o $*.bgf -r -n "KD1: Wall 4 (Bone Cross)" -s 4 1 $(SOURCEDIR)\09214w4.BMP 1 1 1

grd09215.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 1 Dirt" -s 2 1 $(SOURCEDIR)\09215f2.BMP 1 1 1

grd09216.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2 (Middle Area)" -s 2 1 $(SOURCEDIR)\09216f2.BMP 1 1 1

grd09217.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.6 (Bottom Left)" -s 2 1 $(SOURCEDIR)\09217f2.BMP 1 1 1

grd09218.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.3 (Top Right)" -s 2 1 $(SOURCEDIR)\09218f2.BMP 1 1 1

grd09219.bgf:
	makebgf -o $*.bgf -r -n "KD1: FLoor 2.8 (Bottom Right)" -s 2 1 $(SOURCEDIR)\09219f2.BMP 1 1 1

grd09220.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.1 (Top Left)" -s 2 1 $(SOURCEDIR)\09220f2.BMP 1 1 1

grd09221.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.2 (Top Middle)" -s 2 1 $(SOURCEDIR)\09221f2.BMP 1 1 1

grd09222.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.4 (Middle Left)" -s 2 1 $(SOURCEDIR)\09222f2.BMP 1 1 1

grd09223.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.7 (Bottom Middle)" -s 2 1 $(SOURCEDIR)\09223f2.BMP 1 1 1

grd09224.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 2.5 (Middle Right)" -s 2 1 $(SOURCEDIR)\09224f2.BMP 1 1 1

grd09225.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 3 (Skull in Sand)" -s 2 1 $(SOURCEDIR)\09225f2.BMP 1 1 1

grd09226.bgf:
	makebgf -o $*.bgf -r -n "KD1: Floor 4 (Rib Bones in Sand)" -s 2 1 $(SOURCEDIR)\09226f2.BMP 1 1 1

grd09227.bgf:
	makebgf -o $*.bgf -r -n "KD1: Walltop" -s 4 1 $(SOURCEDIR)\09227w4.BMP 1 1 1

grd09228.bgf:
	makebgf -o $*.bgf -r -n "KD1: Wall Overhang" -s 4 1 $(SOURCEDIR)\09228w4.BMP 1 1 1

grd09237.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 1 (Inside)" -s 4 1 $(SOURCEDIR)\09237w4.BMP 1 1 1

grd09238.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 2 (Inside)" -s 4 1 $(SOURCEDIR)\09238w4.BMP 1 1 1

grd09239.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 3 (Inside)" -s 4 1 $(SOURCEDIR)\09239w4.BMP 1 1 1

grd09240.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 4 (Inside)" -s 4 1 $(SOURCEDIR)\09240w4.BMP 1 1 1

grd09241.bgf:
	makebgf -o $*.bgf -r -n "KD4: Fireplace 1 (Anim)" -s 4 3 $(SOURCEDIR)\09241w4A.bmp $(SOURCEDIR)\09241w4B.bmp $(SOURCEDIR)\09241w4C.bmp 3 1 1 1 2 1 3

grd09242.bgf:
	makebgf -o $*.bgf -r -n "KD4: Fireplace 2 (Anim)" -s 4 3 $(SOURCEDIR)\09242w4A.bmp $(SOURCEDIR)\09242w4B.bmp $(SOURCEDIR)\09242w4C.bmp 3 1 1 1 2 1 3

grd09243.bgf:
	makebgf -o $*.bgf -r -n "KD4: Floor 1 (Inside)" -s 2 1 $(SOURCEDIR)\09243f2.BMP 1 1 1

grd09244.bgf:
	makebgf -o $*.bgf -r -n "KD4: Ceiling 1 (Inside)" -s 2 1 $(SOURCEDIR)\09244f2.BMP 1 1 1

grd09245.bgf:
	makebgf -o $*.bgf -r -n "KD4: Floor 2 (Inside)" -s 2 1 $(SOURCEDIR)\09245f2.BMP 1 1 1

grd09246.bgf:
	makebgf -o $*.bgf -r -n "KD4: Tableside W/ Tablecloth" -s 2 1 $(SOURCEDIR)\09246f2.BMP 1 1 1

grd09247.bgf:
	makebgf -o $*.bgf -r -n "KD4: Tabletop W/ Tablecloth" -s 2 1 $(SOURCEDIR)\09247f2.BMP 1 1 1

grd09248.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wood 1" -s 2 1 $(SOURCEDIR)\09248f2.BMP 1 1 1

grd09249.bgf:
	makebgf -o $*.bgf -r -n "KD4: Small Cabinet" -s 4 1 $(SOURCEDIR)\09249w4.BMP 1 1 1

grd09250.bgf:
	makebgf -o $*.bgf -r -n "KD4: Rock Foundation" -s 2 1 $(SOURCEDIR)\09250f2.BMP 1 1 1

grd09251.bgf:
	makebgf -o $*.bgf -r -n "KD4: Corner Foundation" -s 2 1 $(SOURCEDIR)\09251f2.BMP 1 1 1

grd09252.bgf:
	makebgf -o $*.bgf -r -n "KD4: Chimney Bricks" -s 2 1 $(SOURCEDIR)\09252f2.BMP 1 1 1

grd09253.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 5 (w/ Lamp)" -s 4 1 $(SOURCEDIR)\09253w4.BMP 1 1 1

grd09254.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 5 (Flame Anim)" -s 4 3 $(SOURCEDIR)\09254w4A.bmp $(SOURCEDIR)\09254w4B.bmp $(SOURCEDIR)\09254w4C.bmp 3 1 1 1 2 1 3

grd09255.bgf:
	makebgf -o $*.bgf -r -n "KD4: Large Cabinet" -s 4 1 $(SOURCEDIR)\09255w4.BMP 1 1 1

grd09256.bgf:
	makebgf -o $*.bgf -r -n "KD4: Large Cabinet Side" -s 4 1 $(SOURCEDIR)\09256w4.BMP 1 1 1

grd09257.bgf:
	makebgf -o $*.bgf -r -n "KD4: Log Wall 1" -s 4 1 $(SOURCEDIR)\09257w4.BMP 1 1 1

grd09258.bgf:
	makebgf -o $*.bgf -r -n "KD4: Log 1" -s 4 1 $(SOURCEDIR)\09258w4.BMP 1 1 1

grd09259.bgf:
	makebgf -o $*.bgf -r -n "KD4: Log 2" -s 4 1 $(SOURCEDIR)\09259w4.BMP 1 1 1

grd09260.bgf:
	makebgf -o $*.bgf -r -n "KD4: Door Wall 1 (Outside)" -s 4 1 $(SOURCEDIR)\09260w4.BMP 1 1 1

grd09261.bgf:
	makebgf -o $*.bgf -r -n "KD4: Window Wall 1 (Outside)" -s 4 1 $(SOURCEDIR)\09261w4.BMP 1 1 1

grd09262.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 6 (Outside)" -s 4 1 $(SOURCEDIR)\09262w4.BMP 1 1 1

grd09263.bgf:
	makebgf -o $*.bgf -r -n "KD4: Door Wall 2 (Downstairs)" -s 4 1 $(SOURCEDIR)\09263w4.BMP 1 1 1

grd09264.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 7 (Downstairs)" -s 4 1 $(SOURCEDIR)\09264w4.BMP 1 1 1

grd09265.bgf:
	makebgf -o $*.bgf -r -n "KD4: Ceiling 2 (Downstairs)" -s 2 1 $(SOURCEDIR)\09265f2.BMP 1 1 1

grd09266.bgf:
	makebgf -o $*.bgf -r -n "KD4: Post Top" -s 2 1 $(SOURCEDIR)\09266f2.BMP 1 1 1

grd09267.bgf:
	makebgf -o $*.bgf -r -n "KD4: Tapestry 1" -s 4 1 $(SOURCEDIR)\09267w4.BMP 1 1 1

grd09268.bgf:
	makebgf -o $*.bgf -r -n "KD4: Tapestry 2" -s 4 1 $(SOURCEDIR)\09268w4.BMP 1 1 1

grd09269.bgf:
	makebgf -o $*.bgf -r -n "KD4: Ironwork Door" -s 4 1 $(SOURCEDIR)\09269w4.BMP 1 1 1

grd09270.bgf:
	makebgf -o $*.bgf -r -n "KD4: Chimney (Inside)" -s 2 1 $(SOURCEDIR)\09270f2.BMP 1 1 1

grd09271.bgf:
	makebgf -o $*.bgf -r -n "KD4: Floor 3 (Outside)" -s 2 1 $(SOURCEDIR)\09271f2.BMP 1 1 1

grd09272.bgf:
	makebgf -o $*.bgf -r -n "KD4: Floor 4 (Outside)" -s 2 1 $(SOURCEDIR)\09272f2.BMP 1 1 1

grd09273.bgf:
	makebgf -o $*.bgf -r -n "KD4: Door Wall 3" -s 4 1 $(SOURCEDIR)\09273w4.BMP 1 1 1

grd09274.bgf:
	makebgf -o $*.bgf -r -n "KD4: Window Wall 2 (Inside)" -s 4 1 $(SOURCEDIR)\09274w4.BMP 1 1 1

grd09275.bgf:
	makebgf -o $*.bgf -r -n "KD4: Wall 8 (Inside)" -s 4 1 $(SOURCEDIR)\09275w4.BMP 1 1 1

grd09276.bgf:
	makebgf -o $*.bgf -r -n "KD4: Clothesline" -s 4 1 $(SOURCEDIR)\09276w4.BMP 1 1 1

grd09277.bgf:
	makebgf -o $*.bgf -r -n "KD4: Fortress Tower Wall" -s 4 1 $(SOURCEDIR)\09277w4.BMP 1 1 1

grd09278.bgf:
	makebgf -o $*.bgf -r -n "KD4: Fortress Floor" -s 2 1 $(SOURCEDIR)\09278f2.BMP 1 1 1

grd09279.bgf:
	makebgf -o $*.bgf -r -n "KD4: Fortress Low Wall" -s 4 1 $(SOURCEDIR)\09279w4.BMP 1 1 1

grd09280.bgf:
	makebgf -o $*.bgf -r -n "KD4: Gabled Roof Wall" -s 4 1 $(SOURCEDIR)\09280w4.BMP 1 1 1

grd09281.bgf:
	makebgf -o $*.bgf -r -n "KD4: Foundation Flagstones" -s 2 1 $(SOURCEDIR)\09281f2.BMP 1 1 1

grd09282.bgf:
	makebgf -o $*.bgf -r -n "KD4: Window Wall 3 (Attic)" -s 4 1 $(SOURCEDIR)\09282w4.BMP 1 1 1

grd09288.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Post Wall" -s 4 1 $(SOURCEDIR)\09288w4.bmp 1 1 1

grd09289.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Column Face 2" -s 4 1 $(SOURCEDIR)\09289w4.bmp 1 1 1

grd09290.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Door" -s 4 1 $(SOURCEDIR)\09290w4.bmp 1 1 1

grd09291.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bank Door Right" -s 4 1 $(SOURCEDIR)\09291w4.bmp 1 1 1

grd09292.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bank Door Left" -s 4 1 $(SOURCEDIR)\09292w4.bmp 1 1 1

grd09293.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Mural Left" -s 4 1 $(SOURCEDIR)\09293w4.bmp 1 1 1

grd09294.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Mural Right" -s 4 1 $(SOURCEDIR)\09294w4.bmp 1 1 1

grd09295.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bar Mural Center" -s 4 1 $(SOURCEDIR)\09295w4.bmp 1 1 1

grd09296.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Torch Attaches to wall" -s 2 3 $(SOURCEDIR)\09296w2A.bmp $(SOURCEDIR)\09296w2B.bmp $(SOURCEDIR)\09296w2C.bmp 3 1 1 1 2 1 3

grd09297.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Torch Cross [The other part =)]" -s 2 3 $(SOURCEDIR)\09297w2A.bmp $(SOURCEDIR)\09297w2B.bmp $(SOURCEDIR)\09297w2C.bmp 3 1 1 1 2 1 3

grd09298.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Room #'s 1" -s 4 1 $(SOURCEDIR)\09298w4.bmp 1 1 1

grd09299.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Room #'s 2" -s 4 1 $(SOURCEDIR)\09299w4.bmp 1 1 1

grd09300.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Room #'s 3" -s 4 1 $(SOURCEDIR)\09300w4.bmp 1 1 1

grd09301.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Room #'s 4" -s 4 1 $(SOURCEDIR)\09301w4.bmp 1 1 1

grd09302.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Room #'s 5" -s 4 1 $(SOURCEDIR)\09302w4.bmp 1 1 1

grd09303.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Room #'s 6" -s 4 1 $(SOURCEDIR)\09303w4.bmp 1 1 1

grd09304.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Tapestry 1" -s 4 1 $(SOURCEDIR)\09304w4.bmp 1 1 1

grd09305.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Tapestry 2" -s 4 1 $(SOURCEDIR)\09305w4.bmp 1 1 1

grd09306.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bank Portcullis" -s 4 1 $(SOURCEDIR)\09306w4.bmp 1 1 1

grd09307.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bank Securedoor" -s 4 1 $(SOURCEDIR)\09307w4.bmp 1 1 1

grd09308.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 2 Bronze" -s 4 1 $(SOURCEDIR)\09308w4.bmp 1 1 1

grd09309.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 3 Bluegrey" -s 4 1 $(SOURCEDIR)\09309w4.bmp 1 1 1

grd09310.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 4 Orange" -s 4 1 $(SOURCEDIR)\09310w4.bmp 1 1 1

grd09311.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 5 Yellow" -s 4 1 $(SOURCEDIR)\09311w4.bmp 1 1 1

grd09312.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bank Wall Interior" -s 4 1 $(SOURCEDIR)\09312w4.bmp 1 1 1

grd09313.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Bank Stairwall" -s 2 1 $(SOURCEDIR)\09313w2.bmp 1 1 1

grd09314.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Basket Side" -s 4 1 $(SOURCEDIR)\09314w4.bmp 1 1 1

grd09315.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Basket Top" -s 2 1 $(SOURCEDIR)\09315f2.bmp 1 1 1

grd09316.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Stick Box Top" 1 $(SOURCEDIR)\09316f1.bmp 1 1 1

grd09317.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Stick Box Face" -s 2 1 $(SOURCEDIR)\09317w2.bmp 1 1 1

grd09318.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Stick Box Side" -s 2 1 $(SOURCEDIR)\09318w2.bmp 1 1 1

grd09319.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 2 (floor) Bronze" -s 2 1 $(SOURCEDIR)\09319f2.bmp 1 1 1

grd09320.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 3 (floor) Bluegrey" -s 2 1 $(SOURCEDIR)\09320f2.bmp 1 1 1

grd09321.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 4 (floor) Orange" -s 2 1 $(SOURCEDIR)\09321f2.bmp 1 1 1

grd09322.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Box Side" -s 2 1 $(SOURCEDIR)\09322w2.bmp 1 1 1

grd09323.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Box Top" -s 2 1 $(SOURCEDIR)\09323f2.bmp 1 1 1

grd09324.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Lg Box Top" -s 2 1 $(SOURCEDIR)\09324f2.bmp 1 1 1

grd09325.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Lg Box Stamped" -s 2 1 $(SOURCEDIR)\09325f2.bmp 1 1 1

grd09326.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 5 (floor) Yellow" -s 2 1 $(SOURCEDIR)\09326f2.bmp 1 1 1

grd09327.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Inn Wall Skunk" -s 4 1 $(SOURCEDIR)\09327w4.bmp 1 1 1

grd09328.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Store Int Door Sewer" -s 4 1 $(SOURCEDIR)\09328w4.bmp 1 1 1

grd09329.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Sewer Portcullis" -s 4 1 $(SOURCEDIR)\09329w4.bmp 1 1 1

grd09330.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Store Shovel Wall" -s 4 1 $(SOURCEDIR)\09330w4.bmp 1 1 1

grd09331.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Wall 6 Yellow/Pink" -s 4 1 $(SOURCEDIR)\09331w4.bmp 1 1 1

grd09332.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Tabletop Plain" -s 2 1 $(SOURCEDIR)\09332f2.bmp 1 1 1

grd09333.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Table End" -s 2 1 $(SOURCEDIR)\09333w2.bmp 1 1 1

grd09334.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Table Drawers" -s 2 1 $(SOURCEDIR)\09334w2.bmp 1 1 1

grd09335.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Shirtwall 2" -s 4 1 $(SOURCEDIR)\09335w4.bmp 1 1 1

grd09336.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Curtain" -s 4 1 $(SOURCEDIR)\09336w4.bmp 1 1 1

grd09337.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Door" -s 4 1 $(SOURCEDIR)\09337w4.bmp 1 1 1

grd09338.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Guildh13 Wall 1 Stripe" -s 4 1 $(SOURCEDIR)\09338w4.bmp 1 1 1

grd09339.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Guildh13 Wall 2 Stripe" -s 4 1 $(SOURCEDIR)\09339w4.bmp 1 1 1

grd09340.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Shirtwall 1" -s 4 1 $(SOURCEDIR)\09340w4.bmp 1 1 1

grd09341.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Sidetable Top" -s 2 1 $(SOURCEDIR)\09341f2.bmp 1 1 1

grd09342.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Sidetable Side" -s 2 1 $(SOURCEDIR)\09342w2.bmp 1 1 1

grd09343.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set Tailor Tabletop Equip" -s 2 1 $(SOURCEDIR)\09343f2.bmp 1 1 1

grd09344.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Scone Wall" -s 4 3 $(SOURCEDIR)\09344w4A.bmp $(SOURCEDIR)\09344w4B.bmp $(SOURCEDIR)\09344w4C.bmp 3 1 1 1 2 1 3

grd09345.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Set MStore Scone Side" -s 4 3 $(SOURCEDIR)\09345w4A.bmp $(SOURCEDIR)\09345w4B.bmp $(SOURCEDIR)\09345w4C.bmp 3 1 1 1 2 1 3

grd09346.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 3" -s 4 1 $(SOURCEDIR)\09346w4.BMP 1 1 1

grd09347.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 4" -s 4 1 $(SOURCEDIR)\09347w4.BMP 1 1 1

grd09348.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 5" -s 4 1 $(SOURCEDIR)\09348w4.BMP 1 1 1

grd09349.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 6" -s 4 1 $(SOURCEDIR)\09349w4.BMP 1 1 1

grd09350.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 7" -s 4 1 $(SOURCEDIR)\09350w4.BMP 1 1 1

grd09351.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 8" -s 4 1 $(SOURCEDIR)\09351w4.BMP 1 1 1

grd09352.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Shirtwall 9" -s 4 1 $(SOURCEDIR)\09352w4.BMP 1 1 1

grd09353.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith 1Right Weirdbird Wall" -s 4 1 $(SOURCEDIR)\09353w4.BMP 1 1 1

grd09354.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith 1Center Weirdbird Wall" -s 4 1 $(SOURCEDIR)\09354w4.BMP 1 1 1

grd09355.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith 1Left Weirdbird Wall" -s 4 1 $(SOURCEDIR)\09355w4.BMP 1 1 1

grd09356.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith Bird Mural Wall" -s 4 1 $(SOURCEDIR)\09356w4.BMP 1 1 1

grd09357.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith Door Marked" -s 4 1 $(SOURCEDIR)\09357w4.BMP 1 1 1

grd09358.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith Door Unmarked" -s 4 1 $(SOURCEDIR)\09358w4.BMP 1 1 1

grd09359.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith Portcullis" -s 4 1 $(SOURCEDIR)\09359w4.BMP 1 1 1

grd09360.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith SwordWall 1" -s 4 1 $(SOURCEDIR)\09360w4.BMP 1 1 1

grd09361.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith SwordWall 2" -s 4 1 $(SOURCEDIR)\09361w4.BMP 1 1 1

grd09362.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith Cabinet Face" -s 4 1 $(SOURCEDIR)\09362w4.BMP 1 1 1

grd09363.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith Cabinet Side" -s 4 1 $(SOURCEDIR)\09363w4.BMP 1 1 1

grd09364.bgf:
	makebgf -o $*.bgf -r -n "Set BSmith ToolWall" -s 4 1 $(SOURCEDIR)\09364w4.BMP 1 1 1

grd09365.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks DoorWall" -s 4 1 $(SOURCEDIR)\09365w4.BMP 1 1 1

grd09366.bgf:
	makebgf -o $*.bgf -r -n "Set HoH Statue Border1 (Temp)" -s 4 1 $(SOURCEDIR)\09366w4.BMP 1 1 1

grd09367.bgf:
	makebgf -o $*.bgf -r -n "Set Military Locker Face" -s 4 1 $(SOURCEDIR)\09367w4.BMP 1 1 1

grd09368.bgf:
	makebgf -o $*.bgf -r -n "Set Military Locker Side" -s 4 1 $(SOURCEDIR)\09368w4.BMP 1 1 1

grd09369.bgf:
	makebgf -o $*.bgf -r -n "Set Outdoor Stripe Wall 3" -s 4 1 $(SOURCEDIR)\09369w4.BMP 1 1 1

grd09370.bgf:
	makebgf -o $*.bgf -r -n "Set OUtdoor Stripe Wall 4" -s 4 1 $(SOURCEDIR)\09370w4.BMP 1 1 1

grd09371.bgf:
	makebgf -o $*.bgf -r -n "Set Outdoor Floor Blue (For Stripe Wall 3&4)" -s 4 1 $(SOURCEDIR)\09371w4.BMP 1 1 1

grd09372.bgf:
	makebgf -o $*.bgf -r -n "Set Inn Interior Door" -s 4 1 $(SOURCEDIR)\09372w4.BMP 1 1 1

grd09373.bgf:
	makebgf -o $*.bgf -r -n "Set Military Pillar Wall" -s 4 1 $(SOURCEDIR)\09373w4.BMP 1 1 1

grd09374.bgf:
	makebgf -o $*.bgf -r -n "Set Military Outer Gate" -s 4 1 $(SOURCEDIR)\09374w4.BMP 1 1 1

grd09375.bgf:
	makebgf -o $*.bgf -r -n "Set Military Hallway 1" -s 4 1 $(SOURCEDIR)\09375w4.BMP 1 1 1

grd09376.bgf:
	makebgf -o $*.bgf -r -n "Set Military Hallway 2" -s 4 1 $(SOURCEDIR)\09376w4.BMP 1 1 1

grd09377.bgf:
	makebgf -o $*.bgf -r -n "Set HoH Target1 Front" -s 3 1 $(SOURCEDIR)\09377w4.BMP 1 1 1

grd09378.bgf:
	makebgf -o $*.bgf -r -n "Set HoH Target1 Back" -s 3 1 $(SOURCEDIR)\09378w4.BMP 1 1 1

grd09379.bgf:
	makebgf -o $*.bgf -r -n "Set HoH Target1 Prop" -s 3 1 $(SOURCEDIR)\09379w4.BMP 1 1 1

grd09380.bgf:
	makebgf -o $*.bgf -r -n "Set HoH Target2 Front" -s 3 1 $(SOURCEDIR)\09380w3.BMP 1 1 1

grd09381.bgf:
	makebgf -o $*.bgf -r -n "Set HoH Target2 Back" -s 3 1 $(SOURCEDIR)\09381w3.BMP 1 1 1

grd09382.bgf:
	makebgf -o $*.bgf -r -n "Set Guard Towers Portcullis" -s 4 1 $(SOURCEDIR)\09382w4.BMP 1 1 1

grd09383.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 1 Regiment Wall" -s 4 1 $(SOURCEDIR)\09383w4.BMP 1 1 1

grd09384.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 2 Regiment Wall" -s 4 1 $(SOURCEDIR)\09384w4.BMP 1 1 1

grd09385.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 3 Regiment Wall" -s 4 1 $(SOURCEDIR)\09385w4.BMP 1 1 1

grd09386.bgf:
	makebgf -o $*.bgf -r -n "KE4: Idol Head Top" -s 1 1 $(SOURCEDIR)\09386w1.BMP 1 1 1

grd09387.bgf:
	makebgf -o $*.bgf -r -n "KE4: Idol Head Bottom" -s 1 1 $(SOURCEDIR)\09387w1.BMP 1 1 1

grd09388.bgf:
	makebgf -o $*.bgf -r -n "KE4: Obelisk 1" -s 1 1 $(SOURCEDIR)\09388w1.BMP 1 1 1

grd09389.bgf:
	makebgf -o $*.bgf -r -n "KE4: Obelisk 2" -s 1 1 $(SOURCEDIR)\09389w1.BMP 1 1 1

grd09390.bgf:
	makebgf -o $*.bgf -r -n "KE4: Obelisk 3" -s 1 1 $(SOURCEDIR)\09390w1.BMP 1 1 1

grd09391.bgf:
	makebgf -o $*.bgf -r -n "KE4: Obelisk 4" -s 1 1 $(SOURCEDIR)\09391w1.BMP 1 1 1

grd09392.bgf:
	makebgf -o $*.bgf -r -n "KE4: Idol Stone Plain" -s 1 1 $(SOURCEDIR)\09392f1.BMP 1 1 1

grd09393.bgf:
	makebgf -o $*.bgf -r -n "KE4: Idol Hands Front" -s 1 1 $(SOURCEDIR)\09393w1.BMP 1 1 1

grd09394.bgf:
	makebgf -o $*.bgf -r -n "KE4: Idol Hands Side" -s 1 1 $(SOURCEDIR)\09394w1.BMP 1 1 1

grd09395.bgf:
	makebgf -o $*.bgf -r -n "KE4: Village Rocky Path" -s 2 1 $(SOURCEDIR)\09395f2.BMP 1 1 1

grd09396.bgf:
	makebgf -o $*.bgf -r -n "KE4: Village Dirt (For Houses)" -s 2 1 $(SOURCEDIR)\09396f2.BMP 1 1 1

grd09397.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 4 Regiment Wall" -s 4 1 $(SOURCEDIR)\09397w4.BMP 1 1 1

grd09398.bgf:
	makebgf -o $*.bgf -r -n "Sign Tailor" -s 2 1 $(SOURCEDIR)\09398w2.BMP 1 1 1

grd09399.bgf:
	makebgf -o $*.bgf -r -n "Sign Healer" -s 2 1 $(SOURCEDIR)\09399w2.BMP 1 1 1

grd09400.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 5 Regiment Wall" -s 4 1 $(SOURCEDIR)\09400w4.BMP 1 1 1

grd09401.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 6 Regiment Wall" -s 4 1 $(SOURCEDIR)\09401w4.BMP 1 1 1

grd09402.bgf:
	makebgf -o $*.bgf -r -n "Set Barracks 7 Regiment Wall" -s 4 1 $(SOURCEDIR)\09402w4.BMP 1 1 1

grd09403.bgf:
	makebgf -o $*.bgf -r -n "RT: Front Door Outside" -s 4 1 $(SOURCEDIR)\09403w4.BMP 1 1 1

grd09404.bgf:
	makebgf -o $*.bgf -r -n "RT: Panel Sungod Large" -s 4 1 $(SOURCEDIR)\09404w4.BMP 1 1 1

grd09405.bgf:
	makebgf -o $*.bgf -r -n "RT: Panel 1 Outside" -s 4 1 $(SOURCEDIR)\09405w4.BMP 1 1 1

grd09406.bgf:
	makebgf -o $*.bgf -r -n "RT: Panel 2 Outside" -s 4 1 $(SOURCEDIR)\09406w4.BMP 1 1 1

grd09407.bgf:
	makebgf -o $*.bgf -r -n "RT: Panel 3 Outside" -s 4 1 $(SOURCEDIR)\09407w4.BMP 1 1 1

grd09408.bgf:
	makebgf -o $*.bgf -r -n "RT: Panel 4 Outside" -s 4 1 $(SOURCEDIR)\09408w4.BMP 1 1 1

grd09409.bgf:
	makebgf -o $*.bgf -r -n "RT: Panel 5 Outside" -s 4 1 $(SOURCEDIR)\09409w4.BMP 1 1 1

grd09410.bgf:
	makebgf -o $*.bgf -r -n "RT: Obelisk Panel 1" -s 4 1 $(SOURCEDIR)\09410w4.BMP 1 1 1

grd09411.bgf:
	makebgf -o $*.bgf -r -n "RT: Obelisk Panel 2" -s 4 1 $(SOURCEDIR)\09411w4.BMP 1 1 1

grd09412.bgf:
	makebgf -o $*.bgf -r -n "RT: Obelisk Panel 3" -s 4 1 $(SOURCEDIR)\09412w4.BMP 1 1 1

grd09413.bgf:
	makebgf -o $*.bgf -r -n "RT: Obelisk Panel 4" -s 4 1 $(SOURCEDIR)\09413w4.BMP 1 1 1

grd09414.bgf:
	makebgf -o $*.bgf -r -n "RT: Floor 1 (Inside)" -s 2 1 $(SOURCEDIR)\09414f2.BMP 1 1 1

grd09415.bgf:
	makebgf -o $*.bgf -r -n "RT: Inside Bricks" -s 2 1 $(SOURCEDIR)\09415f2.BMP 1 1 1

grd09416.bgf:
	makebgf -o $*.bgf -r -n "RT: Ceiling 1" -s 2 1 $(SOURCEDIR)\09416f2.BMP 1 1 1

grd09417.bgf:
	makebgf -o $*.bgf -r -n "RT: Stone Path to Temple" -s 2 1 $(SOURCEDIR)\09417f2.BMP 1 1 1

grd09418.bgf:
	makebgf -o $*.bgf -r -n "KE1: Wood for Shack" -s 4 1 $(SOURCEDIR)\09418w4.BMP 1 1 1

grd09419.bgf:
	makebgf -o $*.bgf -r -n "KE1: Roof for Shack" -s 2 1 $(SOURCEDIR)\09419f2.BMP 1 1 1

grd09420.bgf:
	makebgf -o $*.bgf -r -n "KE1: Wood Door for Shack" -s 4 1 $(SOURCEDIR)\09420w4.BMP 1 1 1

grd09423.bgf:
	makebgf -o $*.bgf -r -n "Set Tailor Woven Carpet" -s 2 1 $(SOURCEDIR)\09423f2.BMP 1 1 1

grd09424.bgf:
	makebgf -o $*.bgf -r -n "XXX TEST (Set Outdoor Blockwall)" -s 4 1 $(SOURCEDIR)\09424w4.BMP 1 1 1

grd09425.bgf:
	makebgf -o $*.bgf -r -n "Set Guard Towers 1Shieldwall" -s 4 1 $(SOURCEDIR)\09425w4.BMP 1 1 1

grd09426.bgf:
	makebgf -o $*.bgf -r -n "Set Guard Towers 2Shieldwall" -s 4 1 $(SOURCEDIR)\09426w4.BMP 1 1 1

grd09427.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Sign Book" -s 1 1 $(SOURCEDIR)\09427w2.bmp 1 1 1

grd09428.bgf:
	makebgf -o $*.bgf -r -n "Set Housing Doors" -s 4 1 $(SOURCEDIR)\09428w4.BMP 1 1 1

grd09503.bgf:
	makebgf -o $*.bgf -r -n "TOS-B FWL" -s 2 1 $(SOURCEDIR)\09503w2.BMP 1 1 1

grd09504.bgf:
	makebgf -o $*.bgf -r -n "TOS-B FWC" -s 2 1 $(SOURCEDIR)\09504w2.BMP 1 1 1

grd09505.bgf:
	makebgf -o $*.bgf -r -n "TOS-B FWL W/DOOR" -s 2 1 $(SOURCEDIR)\09505w2.BMP 1 1 1

grd09506.bgf:
	makebgf -o $*.bgf -r -n "TOS-B WALL" -s 2 1 $(SOURCEDIR)\09506w2.BMP 1 1 1

grd09507.bgf:
	makebgf -o $*.bgf -r -n "TOS-ROOF-ORN W/TRANS" -s 4 1 $(SOURCEDIR)\09507w4.BMP 1 1 1

grd09508.bgf:
	makebgf -o $*.bgf -r -n "TOS-D-FRONT W" -s 2 1 $(SOURCEDIR)\09508w2.BMP 1 1 1

grd09509.bgf:
	makebgf -o $*.bgf -r -n "TOS-D FRONT W/DOOR" -s 2 1 $(SOURCEDIR)\09509w2.BMP 1 1 1

grd09510.bgf:
	makebgf -o $*.bgf -r -n "TOS-D WAL" -s 2 1 $(SOURCEDIR)\09510w2.BMP 1 1 1

grd09511.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FNTW L&R M&R" -s 2 1 $(SOURCEDIR)\09511w2.BMP 1 1 1

grd09512.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FNTW2 L&R M&R" -s 2 1 $(SOURCEDIR)\09512w2.BMP 1 1 1

grd09513.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FNTW CENT" -s 2 1 $(SOURCEDIR)\09513w2.BMP 1 1 1

grd09514.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FLR&CEI" -s 1 1 $(SOURCEDIR)\09514f1.BMP 1 1 1

grd09515.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG FNTW CENCOR" -s 4 1 $(SOURCEDIR)\09515w4.BMP 1 1 1

grd09516.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-C FNTW L&R M" -s 2 1 $(SOURCEDIR)\09516w2.BMP 1 1 1

grd09517.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-C FNTW CENT" -s 2 1 $(SOURCEDIR)\09517w2.BMP 1 1 1

grd09518.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-C FLR&CEI" -s 1 1 $(SOURCEDIR)\09518f1.BMP 1 1 1

grd09519.bgf:
	makebgf -o $*.bgf -r -n "TOS-E ALL WALLS NO DR" -s 2 1 $(SOURCEDIR)\09519w2.BMP 1 1 1

grd09520.bgf:
	makebgf -o $*.bgf -r -n "TOS-E WALL W/DOOR" -s 2 1 $(SOURCEDIR)\09520w2.BMP 1 1 1

grd09521.bgf:
	makebgf -o $*.bgf -r -n "TOS-E WALL" -s 2 1 $(SOURCEDIR)\09521w2.BMP 1 1 1

grd09522.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-E ROOF" -s 2 1 $(SOURCEDIR)\09522w2.BMP 1 1 1

grd09523.bgf:
	makebgf -o $*.bgf -r -n "TOS-H WALL W/DOOR" -s 2 1 $(SOURCEDIR)\09523w2.BMP 1 1 1

grd09524.bgf:
	makebgf -o $*.bgf -r -n "TOS-H WALL W/OUT DOOR" -s 2 1 $(SOURCEDIR)\09524w2.BMP 1 1 1

grd09525.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-H WALL" -s 2 1 $(SOURCEDIR)\09525w2.BMP 1 1 1

grd09526.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-F FNTW L&R M&R" -s 2 1 $(SOURCEDIR)\09526w2.BMP 1 1 1

grd09527.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-F FNTW L&R M&R2" -s 2 1 $(SOURCEDIR)\09527w2.BMP 1 1 1

grd09528.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-F FNTW CENT" -s 2 1 $(SOURCEDIR)\09528w2.BMP 1 1 1

grd09529.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-F FLR&CEI" -s 1 1 $(SOURCEDIR)\09529f1.BMP 1 1 1

grd09530.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-F BCKW M&R" -s 2 1 $(SOURCEDIR)\09530w2.BMP 1 1 1

grd09531.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-G FLR&CEI" -s 1 1 $(SOURCEDIR)\09531f1.BMP 1 1 1

grd09532.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG G FNTW CENT&S M&R" -s 2 1 $(SOURCEDIR)\09532w2.BMP 1 1 1

grd09533.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL K FNT L&R M&R" -s 2 1 $(SOURCEDIR)\09533w2.BMP 1 1 1

grd09534.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL K SIDE L&R M&R" -s 2 1 $(SOURCEDIR)\09534w2.BMP 1 1 1

grd09535.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL K CENT" -s 2 1 $(SOURCEDIR)\09535w2.BMP 1 1 1

grd09536.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL L L&R M&R" -s 2 1 $(SOURCEDIR)\09536w2.BMP 1 1 1

grd09537.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL L L&R M" -s 2 1 $(SOURCEDIR)\09537w2.BMP 1 1 1

grd09538.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL L FNT CENT" -s 2 1 $(SOURCEDIR)\09538w2.BMP 1 1 1

grd09539.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL M FNTW L&R M&R" -s 2 1 $(SOURCEDIR)\09539w2.BMP 1 1 1

grd09540.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL M FNTW L&R M" -s 2 1 $(SOURCEDIR)\09540w2.BMP 1 1 1

grd09541.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL M FNTW CENT" -s 2 1 $(SOURCEDIR)\09541w2.BMP 1 1 1

grd09542.bgf:
	makebgf -o $*.bgf -r -n "TOS-WALL M L&R M&R" -s 2 1 $(SOURCEDIR)\09542w2.BMP 1 1 1

grd09543.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-I FNTW WOOD DOOR" -s 2 1 $(SOURCEDIR)\09543w2.BMP 1 1 1

grd09544.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-I FNTW METL DOOR" -s 2 1 $(SOURCEDIR)\09544w2.BMP 1 1 1

grd09545.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-I WALL&CEI" -s 2 1 $(SOURCEDIR)\09545w2.BMP 1 1 1

grd09546.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-I FNTW NO DOOR" -s 2 1 $(SOURCEDIR)\09546w2.BMP 1 1 1

grd09548.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-E CEILING" -s 2 1 $(SOURCEDIR)\09548f2.BMP 1 1 1

grd09549.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-B CEILING" -s 2 1 $(SOURCEDIR)\09549f2.BMP 1 1 1

grd09550.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-D CEILING" -s 2 1 $(SOURCEDIR)\09550f2.BMP 1 1 1

grd09551.bgf:
	makebgf -o $*.bgf -r -n "TOD-BLDG-I CEILING" -s 2 1 $(SOURCEDIR)\09551f2.BMP 1 1 1

grd09552.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A SIDE W" -s 2 1 $(SOURCEDIR)\09552w2.BMP 1 1 1

grd09553.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-C SIDE W" -s 2 1 $(SOURCEDIR)\09553w2.BMP 1 1 1

grd09554.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-F SIDE W" -s 2 1 $(SOURCEDIR)\09554w2.BMP 1 1 1

grd09555.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-G SIDE W" -s 2 1 $(SOURCEDIR)\09555w2.BMP 1 1 1

grd09556.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FNTW CENT R" -s 2 1 $(SOURCEDIR)\09556w2.BMP 1 1 1

grd09557.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FNTW L&R M&R" -s 2 1 $(SOURCEDIR)\09557w2.BMP 1 1 1

grd09558.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A SIDEW BARW" -s 2 1 $(SOURCEDIR)\09558w2.BMP 1 1 1

grd09559.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A BAR TOP" -s 2 1 $(SOURCEDIR)\09559w2.BMP 1 1 1

grd09560.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-A FLR-CEI" -s 2 1 $(SOURCEDIR)\09560f2.BMP 1 1 1

grd09561.bgf:
	makebgf -o $*.bgf -r -n "TOS-FOUNT A SNTBASE" -s 2 1 $(SOURCEDIR)\09561f2.BMP 1 1 1

grd09562.bgf:
	makebgf -o $*.bgf -r -n "TOS-FOUNT-B STNFLR BASE" -s 2 1 $(SOURCEDIR)\09562f2.BMP 1 1 1

grd09563.bgf:
	makebgf -o $*.bgf -r -n "TOS-FOUNT-C SNTWALL 4 STEPS" -s 2 1 $(SOURCEDIR)\09563w2.BMP 1 1 1

grd09564.bgf:
	makebgf -o $*.bgf -r -n "TOS-FOUNT-D SNTFLR 4 BARRIER" -s 2 1 $(SOURCEDIR)\09564f2.BMP 1 1 1

grd09565.bgf:
	makebgf -o $*.bgf -r -n "TOS-FOUNT-E SNTWALL 4 BARRIER" -s 2 1 $(SOURCEDIR)\09565w2.BMP 1 1 1

grd09566.bgf:
	makebgf -o $*.bgf -r -n "TOS-STONEPATH" -s 1 1 $(SOURCEDIR)\09566f1.BMP 1 1 1

grd09567.bgf:
	makebgf -o $*.bgf -r -n "TOS-GRASS" -s 1 1 $(SOURCEDIR)\09567f1.BMP 1 1 1

grd09568.bgf:
	makebgf -o $*.bgf -r -n "tos-aban-intwall H" -s 2 1 $(SOURCEDIR)\09568w2.BMP 1 1 1

grd09569.bgf:
	makebgf -o $*.bgf -r -n "tos-aban-intwalldoor h" -s 2 1 $(SOURCEDIR)\09569w2.BMP 1 1 1

grd09570.bgf:
	makebgf -o $*.bgf -r -n "tos-aban-wallstone h" -s 2 1 $(SOURCEDIR)\09570w2.BMP 1 1 1

grd09571.bgf:
	makebgf -o $*.bgf -r -n "tos-aban-cieling-h" -s 2 1 $(SOURCEDIR)\09571f2.BMP 1 1 1

grd09572.bgf:
	makebgf -o $*.bgf -r -n "tos-aban-floor-h" -s 2 1 $(SOURCEDIR)\09572f2.BMP 1 1 1

grd09573.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-intwall door" -s 2 1 $(SOURCEDIR)\09573w2.BMP 1 1 1

grd09574.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-intwall wo/dor" -s 2 1 $(SOURCEDIR)\09574w2.BMP 1 1 1

grd09575.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-intwall" -s 2 1 $(SOURCEDIR)\09575w2.BMP 1 1 1

grd09576.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-intfloor" -s 2 1 $(SOURCEDIR)\09576f2.BMP 1 1 1

grd09577.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-intceiling" -s 2 1 $(SOURCEDIR)\09577f2.BMP 1 1 1

grd09578.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-partition" -s 4 1 $(SOURCEDIR)\09578w4.BMP 1 1 1

grd09579.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-side chair" -s 4 1 $(SOURCEDIR)\09579w4.BMP 1 1 1

grd09580.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgI-chairtop" -s 4 1 $(SOURCEDIR)\09580w4.BMP 1 1 1

grd09581.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgD-int frntwall" -s 2 1 $(SOURCEDIR)\09581w2.BMP 1 1 1

grd09582.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgD-intfrntwall w/door" -s 2 1 $(SOURCEDIR)\09582w2.BMP 1 1 1

grd09583.bgf:
	makebgf -o $*.bgf -r -n "tos-bldgD-intwall2" -s 4 1 $(SOURCEDIR)\09583w4.BMP 1 1 1

grd09584.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-D INT FLR&CEI" -s 2 1 $(SOURCEDIR)\09584f2.BMP 1 1 1

grd09585.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-D INT FRNT LRGCABS" -s 2 1 $(SOURCEDIR)\09585w2.BMP 1 1 1

grd09586.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-D INT TOP CABS" -s 2 1 $(SOURCEDIR)\09586w2.BMP 1 1 1

grd09587.bgf:
	makebgf -o $*.bgf -r -n "TOS-INT-D INT FRNT SMLCABS" -s 2 1 $(SOURCEDIR)\09587w2.BMP 1 1 1

grd09588.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-D FLOOR" -s 2 1 $(SOURCEDIR)\09588f2.BMP 1 1 1

grd09589.bgf:
	makebgf -o $*.bgf -r -n "TOS-BLDG-D CEILING" -s 2 1 $(SOURCEDIR)\09589f2.BMP 1 1 1

grd09590.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-J A FRNTWALL" -s 2 1 $(SOURCEDIR)\09590w2.BMP 1 1 1

grd09591.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-J B SIDEWALL" -s 2 1 $(SOURCEDIR)\09591w2.BMP 1 1 1

grd09592.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-J C ROOFTILED" -s 2 1 $(SOURCEDIR)\09592f2.BMP 1 1 1

grd09593.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgL-Afrntwall" -s 2 1 $(SOURCEDIR)\09593w2.BMP 1 1 1

grd09594.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-L B FRNTW SIGE MIRR" -s 2 1 $(SOURCEDIR)\09594w2.BMP 1 1 1

grd09595.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-L C FRNTESI CENTD" -s 2 1 $(SOURCEDIR)\09595w2.BMP 1 1 1

grd09596.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-L D FRNTWALL RSIDE" -s 2 1 $(SOURCEDIR)\09596w2.BMP 1 1 1

grd09597.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-L E SIDEWALL" -s 2 1 $(SOURCEDIR)\09597w2.BMP 1 1 1

grd09598.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-L F ROOF" -s 2 1 $(SOURCEDIR)\09598f2.BMP 1 1 1

grd09599.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-M  A FRNTW CENT" -s 2 1 $(SOURCEDIR)\09599w2.BMP 1 1 1

grd09600.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-M B FRNTW SIDE MIR" -s 2 1 $(SOURCEDIR)\09600w2.BMP 1 1 1

grd09601.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-M C SIDEWALL TILED" -s 2 1 $(SOURCEDIR)\09601w2.BMP 1 1 1

grd09602.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-M D ROOF TILED" -s 1 1 $(SOURCEDIR)\09602f1.BMP 1 1 1

grd09603.bgf:
	makebgf -o $*.bgf -r -n "RAZ-GRASSTONE PATH" -s 2 1 $(SOURCEDIR)\09603f2.BMP 1 1 1

grd09604.bgf:
	makebgf -o $*.bgf -r -n "RAZ-STONEPATH" -s 2 1 $(SOURCEDIR)\09604f2.BMP 1 1 1

grd09605.bgf:
	makebgf -o $*.bgf -r -n "RAZ-TREES A" -s 2 1 $(SOURCEDIR)\09605w2.BMP 1 1 1

grd09606.bgf:
	makebgf -o $*.bgf -r -n "RAZ-TREES B" -s 2 1 $(SOURCEDIR)\09606w2.BMP 1 1 1

grd09607.bgf:
	makebgf -o $*.bgf -r -n "RAZ-TREES C" -s 2 1 $(SOURCEDIR)\09607w2.BMP 1 1 1

grd09608.bgf:
	makebgf -o $*.bgf -r -n "RAZ-TREES D" -s 2 1 $(SOURCEDIR)\09608w2.BMP 1 1 1

grd09609.bgf:
	makebgf -o $*.bgf -r -n "RAZ-WALL I CORNER" -s 2 1 $(SOURCEDIR)\09609w2.BMP 1 1 1

grd09610.bgf:
	makebgf -o $*.bgf -r -n "RAZ-WALL I MIDSECTION" -s 2 1 $(SOURCEDIR)\09610w2.BMP 1 1 1

grd09611.bgf:
	makebgf -o $*.bgf -r -n "RAZ-WALL-Q CORNER" -s 2 1 $(SOURCEDIR)\09611w2.BMP 1 1 1

grd09612.bgf:
	makebgf -o $*.bgf -r -n "RAZ-WALL-Q MIDSECTION" -s 2 1 $(SOURCEDIR)\09612w2.BMP 1 1 1

grd09613.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-A ROOF 4 STORSHED" -s 2 1 $(SOURCEDIR)\09613f2.BMP 1 1 1

grd09614.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-A FRNTW W/DOOR" -s 2 1 $(SOURCEDIR)\09614w2.BMP 1 1 1

grd09615.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-A SIDEWALL" -s 2 1 $(SOURCEDIR)\09615w2.BMP 1 1 1

grd09616.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-B ROOF 4 FARM" -s 2 1 $(SOURCEDIR)\09616f2.BMP 1 1 1

grd09617.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-B FRNTW W/OUT DOOR 2B MIR" -s 2 1 $(SOURCEDIR)\09617w2.BMP 1 1 1

grd09618.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-B FRNTW W/DOOR" -s 2 1 $(SOURCEDIR)\09618w2.BMP 1 1 1

grd09619.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-B WALL FOR ADDT." -s 2 1 $(SOURCEDIR)\09619w2.BMP 1 1 1

grd09620.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-B WALL 4 SIDE" -s 2 1 $(SOURCEDIR)\09620w2.BMP 1 1 1

grd09621.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-B CHIMNEY" -s 2 1 $(SOURCEDIR)\09621w2.BMP 1 1 1

grd09622.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-C FRNT 4 GATE" -s 2 1 $(SOURCEDIR)\09622w2.BMP 1 1 1

grd09623.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-C FRNT 4 GATE" -s 2 1 $(SOURCEDIR)\09623w2.BMP 1 1 1

grd09624.bgf:
	makebgf -o $*.bgf -r -n "RAZ-BLDG-G ROOF" -s 2 1 $(SOURCEDIR)\09624f2.BMP 1 1 1

grd09625.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-g frntwl w/out dr" -s 2 1 $(SOURCEDIR)\09625w2.BMP 1 1 1

grd09626.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-g fntwl w/dr" -s 2 1 $(SOURCEDIR)\09626w2.BMP 1 1 1

grd09627.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-g sidewl of entrance" -s 2 1 $(SOURCEDIR)\09627w2.BMP 1 1 1

grd09628.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-g sidewl" -s 2 1 $(SOURCEDIR)\09628w2.BMP 1 1 1

grd09629.bgf:
	makebgf -o $*.bgf -r -n "raz-area-o rockgrnd" -s 2 1 $(SOURCEDIR)\09629f2.BMP 1 1 1

grd09630.bgf:
	makebgf -o $*.bgf -r -n "raz-area-o rockgrnd w/grass" -s 2 1 $(SOURCEDIR)\09630f2.BMP 1 1 1

grd09631.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-a fence " -s 2 1 $(SOURCEDIR)\09631w2.BMP 1 1 1

grd09632.bgf:
	makebgf -o $*.bgf -r -n "raz-area-o fence" -s 2 1 $(SOURCEDIR)\09632w2.BMP 1 1 1

grd09633.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j intwl w/dr" -s 2 1 $(SOURCEDIR)\09633w2.BMP 1 1 1

grd09634.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j intwl side" -s 2 1 $(SOURCEDIR)\09634w2.BMP 1 1 1

grd09635.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j int rearwl" -s 4 1 $(SOURCEDIR)\09635w4.BMP 1 1 1

grd09636.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j sntwood 4 rear corn" -s 2 1 $(SOURCEDIR)\09636w2.BMP 1 1 1

grd09637.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j floorwood" -s 2 1 $(SOURCEDIR)\09637f2.BMP 1 1 1

grd09638.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j cieling" -s 2 1 $(SOURCEDIR)\09638f2.BMP 1 1 1

grd09639.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j side of table" -s 2 1 $(SOURCEDIR)\09639w2.BMP 1 1 1

grd09640.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j tabletop" -s 2 1 $(SOURCEDIR)\09640w2.BMP 1 1 1

grd09641.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j frntcab" -s 2 1 $(SOURCEDIR)\09641w2.BMP 1 1 1

grd09642.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-j sidecab" -s 2 1 $(SOURCEDIR)\09642w2.BMP 1 1 1

grd09643.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-m intdoor" -s 2 1 $(SOURCEDIR)\09643w2.BMP 1 1 1

grd09644.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-m intwl" -s 2 1 $(SOURCEDIR)\09644w2.BMP 1 1 1

grd09645.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-m int cloth 4 tabletop" -s 2 1 $(SOURCEDIR)\09645w2.BMP 1 1 1

grd09646.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-m int cloth 4 sides" -s 2 1 $(SOURCEDIR)\09646w2.BMP 1 1 1

grd09647.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L int frntwl w/door" -s 2 1 $(SOURCEDIR)\09647w2.BMP 1 1 1

grd09648.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L int frntwl w/out dr" -s 2 1 $(SOURCEDIR)\09648w2.BMP 1 1 1

grd09649.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L int sntewls" -s 2 1 $(SOURCEDIR)\09649w2.BMP 1 1 1

grd09650.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L int wood 4 top chair" -s 2 1 $(SOURCEDIR)\09650w2.BMP 1 1 1

grd09651.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L int wood 4 side chair" -s 2 1 $(SOURCEDIR)\09651w2.BMP 1 1 1

grd09652.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L floor" -s 2 1 $(SOURCEDIR)\09652f2.BMP 1 1 1

grd09653.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L ceiling" -s 2 1 $(SOURCEDIR)\09653f2.BMP 1 1 1

grd09654.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-L barrel rack" -s 2 1 $(SOURCEDIR)\09654w2.BMP 1 1 1

grd09655.bgf:
	makebgf -o $*.bgf -r -n "raz-def A outdoor" -s 2 1 $(SOURCEDIR)\09655w2.BMP 1 1 1

grd09656.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF-outdr b door" -s 2 1 $(SOURCEDIR)\09656w2.BMP 1 1 1

grd09657.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF outdr c door" -s 2 1 $(SOURCEDIR)\09657w2.BMP 1 1 1

grd09658.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF-outdr d tallwall w/win" -s 2 1 $(SOURCEDIR)\09658w2.BMP 1 1 1

grd09659.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF-outdr e tallwl w/door" -s 2 1 $(SOURCEDIR)\09659w2.BMP 1 1 1

grd09660.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF-outdr f top of walls" -s 2 1 $(SOURCEDIR)\09660w2.BMP 1 1 1

grd09661.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF-outdr g wall w/round win" -s 2 1 $(SOURCEDIR)\09661w2.BMP 1 1 1

grd09662.bgf:
	makebgf -o $*.bgf -r -n "raz-DEF-outdr h repeat wall" -s 4 1 $(SOURCEDIR)\09662w4.BMP 1 1 1

grd09663.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-h A wl w/ stainedglss" -s 2 1 $(SOURCEDIR)\09663w2.BMP 1 1 1

grd09664.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-h B colum 4 corner of bldg" -s 2 1 $(SOURCEDIR)\09664w2.BMP 1 1 1

grd09665.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-h C wall w/door" -s 2 1 $(SOURCEDIR)\09665w2.BMP 1 1 1

grd09666.bgf:
	makebgf -o $*.bgf $(WALL) -n "Raza Clock Face" -s 2 12 \
	$(SOURCEDIR)\09666w2l.bmp \
	$(SOURCEDIR)\09666w2a.bmp \
	$(SOURCEDIR)\09666w2b.bmp \
	$(SOURCEDIR)\09666w2c.bmp \
	$(SOURCEDIR)\09666w2d.bmp \
	$(SOURCEDIR)\09666w2e.bmp \
	$(SOURCEDIR)\09666w2f.bmp \
	$(SOURCEDIR)\09666w2g.bmp \
	$(SOURCEDIR)\09666w2h.bmp \
	$(SOURCEDIR)\09666w2i.bmp \
	$(SOURCEDIR)\09666w2j.bmp \
	$(SOURCEDIR)\09666w2k.bmp \
	48 \
	1 1 1 2 1 3 1 4 1 5 1 6 1 7 1 8 1 9 1 10 1 11 1 12 \
	1 1 1 2 1 3 1 4 1 5 1 6 1 7 1 8 1 9 1 10 1 11 1 12 \
	1 12 1 11 1 10 1 9 1 8 1 7 1 6 1 5 1 4 1 3 1 2 1 1 \
	1 12 1 11 1 10 1 9 1 8 1 7 1 6 1 5 1 4 1 3 1 2 1 1 

grd09667.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-h E tower w/out clck" -s 2 1 $(SOURCEDIR)\09667w2.BMP 1 1 1

grd09668.bgf:
	makebgf -o $*.bgf -r -n "raz_bldg-h F top wall small" -s 2 1 $(SOURCEDIR)\09668w2.BMP 1 1 1

grd09669.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-h top of wall large" -s 2 1 $(SOURCEDIR)\09669w2.BMP 1 1 1

grd09670.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-h  tall colum eiter side" -s 2 1 $(SOURCEDIR)\09670w2.BMP 1 1 1

grd09671.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-i A front wall" -s 2 1 $(SOURCEDIR)\09671w2.BMP 1 1 1

grd09672.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-i B side wall" -s 2 1 $(SOURCEDIR)\09672w2.BMP 1 1 1

grd09673.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-i c frmtwall w/door" -s 2 1 $(SOURCEDIR)\09673w2.BMP 1 1 1

grd09674.bgf:
	makebgf -o $*.bgf -r -n "ra-bldg-i repeating wall" -s 2 1 $(SOURCEDIR)\09674w2.BMP 1 1 1

grd09675.bgf:
	makebgf -o $*.bgf -r -n "raz-bldg-i fence " -s 2 1 $(SOURCEDIR)\09675w2.BMP 1 1 1

grd09676.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a ext wall" -s 4 1 $(SOURCEDIR)\09676w4.BMP 1 1 1

grd09677.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a b ext door" -s 4 1 $(SOURCEDIR)\09677w4.BMP 1 1 1

grd09678.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a c ext wall simp" -s 2 1 $(SOURCEDIR)\09678w2.BMP 1 1 1

grd09679.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a d ext wall" -s 4 1 $(SOURCEDIR)\09679w4.BMP 1 1 1

grd09680.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a e ext wall gate" -s 4 1 $(SOURCEDIR)\09680w4.BMP 1 1 1

grd09681.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a f ext floor" -s 2 1 $(SOURCEDIR)\09681f2.BMP 1 1 1

grd09682.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a ga fountain side" -s 2 1 $(SOURCEDIR)\09682w2.BMP 1 1 1

grd09683.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a ga fount top" -s 2 1 $(SOURCEDIR)\09683f2.BMP 1 1 1

grd09684.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a ga fount water" -s 2 3 \
             $(SOURCEDIR)\09684f2a.BMP \
             $(SOURCEDIR)\09684f2b.BMP \
             $(SOURCEDIR)\09684f2c.BMP 3 1 1 1 2 1 3

grd09685.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a h ceiling" -s 1 1 $(SOURCEDIR)\09685f1.BMP 1 1 1

grd09686.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a aa intwall" -s 4 1 $(SOURCEDIR)\09686w4.BMP 1 1 1

grd09687.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a ab intwall w/doorway" -s 4 1 $(SOURCEDIR)\09687w4.BMP 1 1 1

grd09688.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a cc table top" -s 2 1 $(SOURCEDIR)\09688f2.BMP 1 1 1

grd09689.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a dd table side" -s 4 1 $(SOURCEDIR)\09689w4.BMP 1 1 1

grd09690.bgf:
	makebgf -o $*.bgf -r -n "nec bldg a ee int flor" -s 2 1 $(SOURCEDIR)\09690f2.BMP 1 1 1

grd09691.bgf:
	makebgf -o $*.bgf -r -n "nec cem area c a sntwall" -s 2 1 $(SOURCEDIR)\09691w2.BMP 1 1 1

grd09692.bgf:
	makebgf -o $*.bgf -r -n "nec cem area c b iron fence" -s 4 1 $(SOURCEDIR)\09692w4.BMP 1 1 1

grd09693.bgf:
	makebgf -o $*.bgf -r -n "nec cem area c c dirtfloor" -s 2 1 $(SOURCEDIR)\09693f2.BMP 1 1 1

grd09694.bgf:
	makebgf -o $*.bgf -r -n "nec roc area d a natroc" -s 2 1 $(SOURCEDIR)\09694w2.BMP 1 1 1

grd09695.bgf:
	makebgf -o $*.bgf -r -n "nec roc area d b darknatroc" -s 2 1 $(SOURCEDIR)\09695w2.BMP 1 1 1

grd09696.bgf:
	makebgf -o $*.bgf -r -n "nec roc area d c greenpuss" -s 2 1 $(SOURCEDIR)\09696w2.BMP 1 1 1

grd09697.bgf:
	makebgf -o $*.bgf -r -n "nec roc area d d pathflr from g9" -s 2 1 $(SOURCEDIR)\09697f2.BMP 1 1 1

grd09698.bgf:
	makebgf -o $*.bgf -r -n "nec roc area e a natroc downtown" -s 4 1 $(SOURCEDIR)\09698w4.BMP 1 1 1

grd09699.bgf:
	makebgf -o $*.bgf -r -n "nec-areaE-gate to maze" -s 4 1 $(SOURCEDIR)\09699w4.BMP 1 1 1

grd09700.bgf:
	makebgf -o $*.bgf -r -n "nec-areaE-c ceiling" -s 4 1 $(SOURCEDIR)\09700w4.BMP 1 1 1

grd09701.bgf:
	makebgf -o $*.bgf -r -n "nec-areaG-mazewall2" -s 2 1 $(SOURCEDIR)\09701w2.BMP 1 1 1

grd09702.bgf:
	makebgf -o $*.bgf -r -n "nec-areaG-skel/spiderpat2" -s 2 1 $(SOURCEDIR)\09702w2.BMP 1 1 1

grd09703.bgf:
	makebgf -o $*.bgf -r -n "nec-area-i darkroom" -s 2 1 $(SOURCEDIR)\09703w2.BMP 1 1 1

grd09704.bgf:
	makebgf -o $*.bgf -r -n "nec-area-i darkroom wall" -s 2 1 $(SOURCEDIR)\09704w2.BMP 1 1 1

grd09705.bgf:
	makebgf -o $*.bgf -r -n "nec-area-f hall texture" -s 4 1 $(SOURCEDIR)\09705w4.BMP 1 1 1

grd09706.bgf:
	makebgf -o $*.bgf -r -n "nec-area-f door 4 ext" -s 4 1 $(SOURCEDIR)\09706w4.BMP 1 1 1

grd09707.bgf:
	makebgf -o $*.bgf -r -n "nec-area-f dead grass" -s 2 1 $(SOURCEDIR)\09707f2.BMP 1 1 1

grd09708.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h wall text int" -s 4 1 $(SOURCEDIR)\09708w4.BMP 1 1 1

grd09709.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h wall text int2" -s 4 1 $(SOURCEDIR)\09709w4.BMP 1 1 1

grd09710.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h tabletop" -s 2 1 $(SOURCEDIR)\09710w2.BMP 1 1 1

grd09711.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h tableside" -s 2 1 $(SOURCEDIR)\09711w2.BMP 1 1 1

grd09712.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h wall text w/door" -s 4 1 $(SOURCEDIR)\09712w4.BMP 1 1 1

grd09713.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h floor" -s 2 1 $(SOURCEDIR)\09713f2.BMP 1 1 1

grd09714.bgf:
	makebgf -o $*.bgf -r -n "nec-bldg-h cieling" -s 2 1 $(SOURCEDIR)\09714f2.BMP 1 1 1

grd09715.bgf:
	makebgf -o $*.bgf -r -n "raza-bldgD-intA winwall" -s 2 1 $(SOURCEDIR)\09715w2.BMP 1 1 1

grd09716.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intB door" -s 2 1 $(SOURCEDIR)\09716w2.BMP 1 1 1

grd09717.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intC wall" -s 2 1 $(SOURCEDIR)\09717w2.BMP 1 1 1

grd09718.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intD weapons" -s 2 1 $(SOURCEDIR)\09718w2.BMP 1 1 1

grd09719.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intE halfwall" -s 2 1 $(SOURCEDIR)\09719w2.BMP 1 1 1

grd09720.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intF forge top" -s 2 1 $(SOURCEDIR)\09720f2.BMP 1 1 1

grd09721.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intG-flagstonflr" -s 2 1 $(SOURCEDIR)\09721f2.BMP 1 1 1

grd09722.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intH ceiling" -s 2 1 $(SOURCEDIR)\09722f2.BMP 1 1 1

grd09723.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgD-intI wall w/beams" -s 2 1 $(SOURCEDIR)\09723w2.BMP 1 1 1

grd09724.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgf-intA curvedwin" -s 2 1 $(SOURCEDIR)\09724w2.BMP 1 1 1

grd09725.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgf-intB cabs" -s 4 1 $(SOURCEDIR)\09725w4.BMP 1 1 1

grd09726.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgf-intC counterside" -s 2 1 $(SOURCEDIR)\09726w2.BMP 1 1 1

grd09727.bgf:
	makebgf -o $*.bgf -r -n "raz-bldgf-intD countertop" -s 2 1 $(SOURCEDIR)\09727f2.BMP 1 1 1

grd09734.bgf:
	makebgf -o $*.bgf -r -n "nechall wall 1" -s 2 1 $(SOURCEDIR)\09734w2.BMP 1 1 1

grd09735.bgf:
	makebgf -o $*.bgf -r -n "nechall wall 2" -s 2 1 $(SOURCEDIR)\09735w2.BMP 1 1 1

grd09736.bgf:
	makebgf -o $*.bgf -r -n "nechall wall 3" -s 2 1 $(SOURCEDIR)\09736w2.BMP 1 1 1

grd09737.bgf:
	makebgf -o $*.bgf -r -n "nechall wall 4" -s 2 1 $(SOURCEDIR)\09737w2.BMP 1 1 1

grd09738.bgf:
	makebgf -o $*.bgf -r -n "nechall floor 1" -s 1 1 $(SOURCEDIR)\09738f1.BMP 1 1 1

grd09739.bgf:
	makebgf -o $*.bgf -r -n "nechall floor 2" -s 1 1 $(SOURCEDIR)\09739f1.BMP 1 1 1

grd09740.bgf:
	makebgf -o $*.bgf -r -n "nechall floor 3" -s 1 1 $(SOURCEDIR)\09740f1.BMP 1 1 1

grd09741.bgf:
	makebgf -o $*.bgf -r -n "razmus-a" -s 2 1 $(SOURCEDIR)\09741w2.BMP 1 1 1

grd09742.bgf:
	makebgf -o $*.bgf -r -n "razmus-d" -s 2 1 $(SOURCEDIR)\09742w2.BMP 1 1 1

grd09743.bgf:
	makebgf -o $*.bgf -r -n "razmus-f" -s 2 1 $(SOURCEDIR)\09743w2.BMP 1 1 1

grd09744.bgf:
	makebgf -o $*.bgf -r -n "razmus-g" -s 2 1 $(SOURCEDIR)\09744w2.BMP 1 1 1

grd09745.bgf:
	makebgf -o $*.bgf -r -n "razmus-c" -s 2 1 $(SOURCEDIR)\09745w2.BMP 1 1 1

grd09746.bgf:
	makebgf -o $*.bgf -r -n "razmus-h" -s 2 1 $(SOURCEDIR)\09746w2.BMP 1 1 1

grd09753.bgf:
	makebgf -o $*.bgf -r -n "nec-hall green ceiling" -s 2 1 $(SOURCEDIR)\09753f2.BMP 1 1 1

grd09765.bgf:
	makebgf -o $*.bgf -r -n "razmus-e" -s 2 1 $(SOURCEDIR)\09765w2.BMP 1 1 1

grd09766.bgf:
	makebgf -o $*.bgf -r -n "razmus-i" -s 2 1 $(SOURCEDIR)\09766w2.BMP 1 1 1

grd09767.bgf:
	makebgf -o $*.bgf -r -n "razmus-b" -s 2 1 $(SOURCEDIR)\09767w2.BMP 1 1 1

grd09768.bgf:
	makebgf -o $*.bgf -r -n "tos inn b" -s 2 1 $(SOURCEDIR)\09768w2.BMP 1 1 1

grd09769.bgf:
	makebgf -o $*.bgf -r -n "tos inn c" -s 2 1 $(SOURCEDIR)\09769w2.BMP 1 1 1

grd09770.bgf:
	makebgf -o $*.bgf -r -n "tos inn d" -s 2 1 $(SOURCEDIR)\09770w2.BMP 1 1 1

grd09771.bgf:
	makebgf -o $*.bgf -r -n "tos inn e" -s 2 1 $(SOURCEDIR)\09771w2.BMP 1 1 1

grd09772.bgf:
	makebgf -o $*.bgf -r -n "tos inn f" -s 2 1 $(SOURCEDIR)\09772f2.BMP 1 1 1

grd09773.bgf:
	makebgf -o $*.bgf -r -n "tos inn g" -s 2 1 $(SOURCEDIR)\09773w2.BMP 1 1 1

grd09774.bgf:
	makebgf -o $*.bgf -r -n "tos grey undamaged arch" -s 2 1 $(SOURCEDIR)\09774w2.BMP 1 1 1

grd09776.bgf:
	makebgf -o $*.bgf -r -n "Twisted Woods basic" -s 2 1 $(SOURCEDIR)\10000w2.BMP 1 1 1

grd09777.bgf:
	makebgf -o $*.bgf -r -n "Twisted Woods edge" -s 2 1 $(SOURCEDIR)\10001w2.BMP 1 1 1

grd10000.bgf:
	makebgf -o $*.bgf -r -n "Twisted Woods basic" -s 2 1 $(SOURCEDIR)\10000w2.BMP 1 1 1

grd10001.bgf:
	makebgf -o $*.bgf -r -n "Twisted Woods edge" -s 2 1 $(SOURCEDIR)\10001w2.BMP 1 1 1

grd10002.bgf:
	makebgf -o $*.bgf -r -n "Jas inn front door" -s 2 1 $(SOURCEDIR)\10002w2.BMP 1 1 1

grd10003.bgf:
	makebgf -o $*.bgf -r -n "Jas inn side wall" -s 2 1 $(SOURCEDIR)\10003w2.BMP 1 1 1

grd10004.bgf:
	makebgf -o $*.bgf -r -n "Jas inn rep. wall" -s 2 1 $(SOURCEDIR)\10004w2.BMP 1 1 1

grd10005.bgf:
	makebgf -o $*.bgf -r -n "Jas inn torch arm" -s 2 1 $(SOURCEDIR)\10005w2.BMP 1 1 1

grd10006.bgf:
	makebgf -o $*.bgf -r -n "Jas inn roof" -s 2 1 $(SOURCEDIR)\10006w2.BMP 1 1 1

grd10007.bgf:
	makebgf -o $*.bgf -r -n "Jas inn stair side" -s 2 1 $(SOURCEDIR)\10007w2.BMP 1 1 1

grd10008.bgf:
	makebgf -o $*.bgf -r -n "Jas inn stair steps" -s 2 1 $(SOURCEDIR)\10008w2.BMP 1 1 1

grd10009.bgf:
	makebgf -o $*.bgf -r -n "Jas rock riser" -s 2 1 $(SOURCEDIR)\10009w2.BMP 1 1 1

grd10010.bgf:
	makebgf -o $*.bgf -r -n "Jas bank door" -s 2 1 $(SOURCEDIR)\10010w2.BMP 1 1 1

grd10011.bgf:
	makebgf -o $*.bgf -r -n "Jas bank griffon" -s 2 1 $(SOURCEDIR)\10011w2.BMP 1 1 1

grd10012.bgf:
	makebgf -o $*.bgf -r -n "Jas bank end piece" -s 2 1 $(SOURCEDIR)\10012w2.BMP 1 1 1

grd10013.bgf:
	makebgf -o $*.bgf -r -n "Jas bank blank" -s 2 1 $(SOURCEDIR)\10013w2.BMP 1 1 1

grd10014.bgf:
	makebgf -o $*.bgf -r -n "Jas inn sign" -s 2 1 $(SOURCEDIR)\10014w2.BMP 1 1 1

grd10020.bgf:
	makebgf -o $*.bgf -r -n "Jas elder door" -s 2 1 $(SOURCEDIR)\10020w2.BMP 1 1 1

grd10021.bgf:
	makebgf -o $*.bgf -r -n "Jas elder maze" -s 2 1 $(SOURCEDIR)\10021w2.BMP 1 1 1

grd10022.bgf:
	makebgf -o $*.bgf -r -n "Jas elder corner" -s 2 1 $(SOURCEDIR)\10022w2.BMP 1 1 1

grd10023.bgf:
	makebgf -o $*.bgf -r -n "Jas elder blank" -s 2 1 $(SOURCEDIR)\10023w2.BMP 1 1 1

grd10030.bgf:
	makebgf -o $*.bgf -r -n "Jas sewer door" -s 2 1 $(SOURCEDIR)\10030w2.BMP 1 1 1

grd10031.bgf:
	makebgf -o $*.bgf -r -n "Jas sewer drain-L" -s 2 1 $(SOURCEDIR)\10031w2.BMP 1 1 1

grd10032.bgf:
	makebgf -o $*.bgf -r -n "Jas sewer drain-R" -s 2 1 $(SOURCEDIR)\10032w2.BMP 1 1 1

grd10033.bgf:
	makebgf -o $*.bgf -r -n "Jas sewer blank" -s 2 1 $(SOURCEDIR)\10033w2.BMP 1 1 1

grd10040.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern door" -s 2 1 $(SOURCEDIR)\10040w2.BMP 1 1 1

grd10041.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern window" -s 2 1 $(SOURCEDIR)\10041w2.BMP 1 1 1

grd10042.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern window w/ dirt" -s 2 1 $(SOURCEDIR)\10042w2.BMP 1 1 1

grd10043.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern blank" -s 2 1 $(SOURCEDIR)\10043w2.BMP 1 1 1

grd10044.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern blank w/ dirt" -s 2 1 $(SOURCEDIR)\10044w2.BMP 1 1 1

grd10045.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern large wall" -s 2 1 $(SOURCEDIR)\10045w2.BMP 1 1 1

grd10046.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern roof" -s 2 1 $(SOURCEDIR)\10046w2.BMP 1 1 1

grd10047.bgf:
	makebgf -o $*.bgf -r -n "Jas tavern sign" -s 2 1 $(SOURCEDIR)\10047w2.BMP 1 1 1

grd10050.bgf:
	makebgf -o $*.bgf -r -n "Jas adv.hall door" -s 2 1 $(SOURCEDIR)\10050w2.BMP 1 1 1

grd10051.bgf:
	makebgf -o $*.bgf -r -n "Jas adv.hall shields" -s 2 1 $(SOURCEDIR)\10051w2.BMP 1 1 1

grd10052.bgf:
	makebgf -o $*.bgf -r -n "Jas adv.hall back wall" -s 2 1 $(SOURCEDIR)\10052w2.BMP 1 1 1

grd10053.bgf:
	makebgf -o $*.bgf -r -n "Jas adv.hall blank" -s 2 1 $(SOURCEDIR)\10053w2.BMP 1 1 1

grd10054.bgf:
	makebgf -o $*.bgf -r -n "Jas adv.hall roof" -s 2 1 $(SOURCEDIR)\10054w2.BMP 1 1 1

grd10060.bgf:
	makebgf -o $*.bgf -r -n "Jas store door" -s 2 1 $(SOURCEDIR)\10060w2.BMP 1 1 1

grd10061.bgf:
	makebgf -o $*.bgf -r -n "Jas store fruit" -s 2 1 $(SOURCEDIR)\10061w2.BMP 1 1 1

grd10062.bgf:
	makebgf -o $*.bgf -r -n "Jas store fruit-bin side" -s 2 1 $(SOURCEDIR)\10062w2.BMP 1 1 1

grd10063.bgf:
	makebgf -o $*.bgf -r -n "Jas store fruit-bin front" -s 2 1 $(SOURCEDIR)\10063w2.BMP 1 1 1

grd10064.bgf:
	makebgf -o $*.bgf -r -n "Jas store blank" -s 2 1 $(SOURCEDIR)\10064w2.BMP 1 1 1

grd10065.bgf:
	makebgf -o $*.bgf -r -n "Jas store blank-front" -s 2 1 $(SOURCEDIR)\10065w2.BMP 1 1 1

grd10070.bgf:
	makebgf -o $*.bgf -r -n "Jas forge door" -s 2 1 $(SOURCEDIR)\10070w2.BMP 1 1 1

grd10071.bgf:
	makebgf -o $*.bgf -r -n "Jas forge sword-sign" -s 2 1 $(SOURCEDIR)\10071w2.BMP 1 1 1

grd10072.bgf:
	makebgf -o $*.bgf -r -n "Jas forge torch-wall" -s 2 1 $(SOURCEDIR)\10072w2.BMP 1 1 1

grd10073.bgf:
	makebgf -o $*.bgf -r -n "Jas forge blank" -s 2 1 $(SOURCEDIR)\10073w2.BMP 1 1 1

grd10074.bgf:
	makebgf -o $*.bgf -r -n "Jas forge pylon" -s 2 1 $(SOURCEDIR)\10074w2.BMP 1 1 1

grd10080.bgf:
	makebgf -o $*.bgf -r -n "Jas Jaarba's door" -s 2 1 $(SOURCEDIR)\10080w2.BMP 1 1 1

grd10081.bgf:
	makebgf -o $*.bgf -r -n "Jas Jaarba's wall" -s 2 1 $(SOURCEDIR)\10081w2.BMP 1 1 1

grd10090.bgf:
	makebgf -o $*.bgf -r -n "Jas factory door" -s 2 1 $(SOURCEDIR)\10090w2.BMP 1 1 1

grd10093.bgf:
	makebgf -o $*.bgf -r -n "Jas factory barn door" -s 2 1 $(SOURCEDIR)\10093w2.BMP 1 1 1

grd10094.bgf:
	makebgf -o $*.bgf -r -n "Jas factory wall wide" -s 2 1 $(SOURCEDIR)\10094w2.BMP 1 1 1

grd10095.bgf:
	makebgf -o $*.bgf -r -n "Jas factory wall narrow" -s 2 1 $(SOURCEDIR)\10095w2.BMP 1 1 1

grd10096.bgf:
	makebgf -o $*.bgf -r -n "Jas factory roof" -s 2 1 $(SOURCEDIR)\10096w2.BMP 1 1 1

grd10097.bgf:
	makebgf -o $*.bgf -r -n "Jas factory blank" -s 2 1 $(SOURCEDIR)\10097w2.BMP 1 1 1

grd10100.bgf:
	makebgf -o $*.bgf -r -n "Jas schoolmarm's door" -s 2 1 $(SOURCEDIR)\10100w2.BMP 1 1 1

grd10101.bgf:
	makebgf -o $*.bgf -r -n "Jas schoolmarm's small window" -s 2 1 $(SOURCEDIR)\10101w2.BMP 1 1 1

grd10102.bgf:
	makebgf -o $*.bgf -r -n "Jas schoolmarm's window" -s 2 1 $(SOURCEDIR)\10102w2.BMP 1 1 1

grd10103.bgf:
	makebgf -o $*.bgf -r -n "Jas schoolmarm's broken window" -s 2 1 $(SOURCEDIR)\10103w2.BMP 1 1 1

grd10104.bgf:
	makebgf -o $*.bgf -r -n "Jas schoolmarm's roof" -s 2 1 $(SOURCEDIR)\10104w2.BMP 1 1 1

grd10110.bgf:
	makebgf -o $*.bgf -r -n "Jas school door" -s 2 1 $(SOURCEDIR)\10110w2.BMP 1 1 1

grd10111.bgf:
	makebgf -o $*.bgf -r -n "Jas school wall" -s 2 1 $(SOURCEDIR)\10111w2.BMP 1 1 1

grd10120.bgf:
	makebgf -o $*.bgf -r -n "Jas Yevitan's door" -s 2 1 $(SOURCEDIR)\10120w2.BMP 1 1 1

grd10121.bgf:
	makebgf -o $*.bgf -r -n "Jas Yevitan's wall" -s 2 1 $(SOURCEDIR)\10121w2.BMP 1 1 1

grd10130.bgf:
	makebgf -o $*.bgf -r -n "Jas counting-room door" -s 2 1 $(SOURCEDIR)\10130w2.BMP 1 1 1

grd10131.bgf:
	makebgf -o $*.bgf -r -n "Jas counting-room griffon" -s 2 1 $(SOURCEDIR)\10131w2.BMP 1 1 1

grd10132.bgf:
	makebgf -o $*.bgf -r -n "Jas counting-room wall" -s 2 1 $(SOURCEDIR)\10132w2.BMP 1 1 1

grd10140.bgf:
	makebgf -o $*.bgf -r -n "Jas house door" -s 2 1 $(SOURCEDIR)\10140w2.BMP 1 1 1

grd10141.bgf:
	makebgf -o $*.bgf -r -n "Jas house windows" -s 2 1 $(SOURCEDIR)\10141w2.BMP 1 1 1

grd10142.bgf:
	makebgf -o $*.bgf -r -n "Jas house bricks" -s 2 1 $(SOURCEDIR)\10142w2.BMP 1 1 1

grd10143.bgf:
	makebgf -o $*.bgf -r -n "Jas house door narrow" -s 2 1 $(SOURCEDIR)\10143w2.BMP 1 1 1

grd10150.bgf:
	makebgf -o $*.bgf -r -n "Jas arsenal door" -s 2 1 $(SOURCEDIR)\10150w2.BMP 1 1 1

grd10151.bgf:
	makebgf -o $*.bgf -r -n "Jas arsenal window" -s 2 1 $(SOURCEDIR)\10151w2.BMP 1 1 1

grd10152.bgf:
	makebgf -o $*.bgf -r -n "Jas gate medium" -s 2 1 $(SOURCEDIR)\10152w2.BMP 1 1 1

grd10153.bgf:
	makebgf -o $*.bgf -r -n "Jas arsenal blank" -s 2 1 $(SOURCEDIR)\10153w2.BMP 1 1 1

grd10170.bgf:
	makebgf -o $*.bgf -r -n "Jas tenement door" -s 2 1 $(SOURCEDIR)\10170w2.BMP 1 1 1

grd10171.bgf:
	makebgf -o $*.bgf -r -n "Jas tenement wall" -s 2 1 $(SOURCEDIR)\10171w2.BMP 1 1 1

grd10172.bgf:
	makebgf -o $*.bgf -r -n "Jas tenement blank" -s 2 1 $(SOURCEDIR)\10172w2.BMP 1 1 1

grd10173.bgf:
	makebgf -o $*.bgf -r -n "Jas gate large" -s 2 1 $(SOURCEDIR)\10173w2.BMP 1 1 1

grd10174.bgf:
	makebgf -o $*.bgf -r -n "Jas gate wrought iron" -s 2 1 $(SOURCEDIR)\10174w2.BMP 1 1 1

grd10180.bgf:
	makebgf -o $*.bgf -r -n "Jas granary door" -s 2 1 $(SOURCEDIR)\10180w2.BMP 1 1 1

grd10181.bgf:
	makebgf -o $*.bgf -r -n "Jas granary wall front" -s 2 1 $(SOURCEDIR)\10181w2.BMP 1 1 1

grd10182.bgf:
	makebgf -o $*.bgf -r -n "Jas granary wall side" -s 2 1 $(SOURCEDIR)\10182w2.BMP 1 1 1

grd10183.bgf:
	makebgf -o $*.bgf -r -n "Jas granary silo" -s 2 1 $(SOURCEDIR)\10183w2.BMP 1 1 1

grd10190.bgf:
	makebgf -o $*.bgf -r -n "Jas shack door" -s 2 1 $(SOURCEDIR)\10190w2.BMP 1 1 1

grd10191.bgf:
	makebgf -o $*.bgf -r -n "Jas shack wall blank" -s 2 1 $(SOURCEDIR)\10191w2.BMP 1 1 1

grd10194.bgf:
	makebgf -o $*.bgf -r -n "Jas shack roof" -s 2 1 $(SOURCEDIR)\10194w2.BMP 1 1 1

grd10200.bgf:
	makebgf -o $*.bgf -r -n "Jas rock floor" -s 2 1 $(SOURCEDIR)\10200w2.BMP 1 1 1

grd10201.bgf:
	makebgf -o $*.bgf -r -n "Jas rock riser tall" -s 2 1 $(SOURCEDIR)\10201w2.BMP 1 1 1

grd10202.bgf:
	makebgf -o $*.bgf -r -n "Jas quarry stone" -s 1 1 $(SOURCEDIR)\10202w2.BMP 1 1 1

grd10203.bgf:
	makebgf -o $*.bgf -r -n "Jas gate small" -s 1 1 $(SOURCEDIR)\10203w2.BMP 1 1 1

grd10204.bgf:
	makebgf -o $*.bgf -r -n "Jas Wryn's gate" -s 1 1 $(SOURCEDIR)\10204w2.BMP 1 1 1

grd10300.bgf:
	makebgf -o $*.bgf -r -n "Badlands wall light cracks" -s 2 1 $(SOURCEDIR)\10300w2.BMP 1 1 1

grd10301.bgf:
	makebgf -o $*.bgf -r -n "Badlands floor" -s 2 1 $(SOURCEDIR)\10301w2.BMP 1 1 1

grd10302.bgf:
	makebgf -o $*.bgf -r -n "Badlands wall medium cracks" -s 2 1 $(SOURCEDIR)\10302w2.BMP 1 1 1

grd10303.bgf:
	makebgf -o $*.bgf -r -n "Badlands wall dark cracks" -s 2 1 $(SOURCEDIR)\10303w2.BMP 1 1 1

grd11000.bgf:
	makebgf -o $*.bgf -r -n "xgrass" -s 2 1 $(SOURCEDIR)\11000f2.BMP 1 1 1

grd11001.bgf:
	makebgf -o $*.bgf -r -n "MAR-tav-wallback" -s 2 1 $(SOURCEDIR)\11001w2.BMP 1 1 1

grd11002.bgf:
	makebgf -o $*.bgf -r -n "MAR-tav-door" -s 2 1 $(SOURCEDIR)\11002w2.BMP 1 1 1

grd11003.bgf:
	makebgf -o $*.bgf -r -n "MAR-tav-wallfront" -s 2 1 $(SOURCEDIR)\11003w2.BMP 1 1 1

grd11004.bgf:
	makebgf -o $*.bgf -r -n "MAR-tav-wallside" -s 2 1 $(SOURCEDIR)\11004w2.BMP 1 1 1

grd11005.bgf:
	makebgf -o $*.bgf -r -n "MAR-tav-concretepilar" -s 2 1 $(SOURCEDIR)\11005w2.BMP 1 1 1

grd11006.bgf:
	makebgf -o $*.bgf -r -n "MAR-concretetile" -s 2 1 $(SOURCEDIR)\11006f2.BMP 1 1 1

grd11007.bgf:
	makebgf -o $*.bgf -r -n "MAR-inn-door" -s 2 1 $(SOURCEDIR)\11007w2.BMP 1 1 1

grd11008.bgf:
	makebgf -o $*.bgf -r -n "MAR-inn-frontwall" -s 2 1 $(SOURCEDIR)\11008w2.BMP 1 1 1

grd11009.bgf:
	makebgf -o $*.bgf -r -n "MAR-inn-frontside" -s 2 1 $(SOURCEDIR)\11009w2.BMP 1 1 1

grd11010.bgf:
	makebgf -o $*.bgf -r -n "MAR-inn-sideend" -s 2 1 $(SOURCEDIR)\11010w2.BMP 1 1 1

grd11011.bgf:
	makebgf -o $*.bgf -r -n "MAR-inn-sidein" -s 2 1 $(SOURCEDIR)\11011w2.BMP 1 1 1

grd11012.bgf:
	makebgf -o $*.bgf -r -n "MAR-smith-door" -s 2 1 $(SOURCEDIR)\11012w2.BMP 1 1 1

grd11013.bgf:
	makebgf -o $*.bgf -r -n "MAR-smith-front" -s 2 1 $(SOURCEDIR)\11013w2.BMP 1 1 1

grd11014.bgf:
	makebgf -o $*.bgf -r -n "MAR-smith-side" -s 2 1 $(SOURCEDIR)\11014w2.BMP 1 1 1

grd11015.bgf:
	makebgf -o $*.bgf -r -n "MAR-smith-rear" -s 2 1 $(SOURCEDIR)\11015w2.BMP 1 1 1

grd11016.bgf:
	makebgf -o $*.bgf -r -n "MAR-hay" -s 2 1 $(SOURCEDIR)\11016f2.BMP 1 1 1

grd11017.bgf:
	makebgf -o $*.bgf -r -n "MAR-guild-door" -s 2 1 $(SOURCEDIR)\11017w2.BMP 1 1 1

grd11018.bgf:
	makebgf -o $*.bgf -r -n "MAR-guild-doorside" -s 2 1 $(SOURCEDIR)\11018w2.BMP 1 1 1

grd11019.bgf:
	makebgf -o $*.bgf -r -n "MAR-guild-sideA" -s 2 1 $(SOURCEDIR)\11019w2.BMP 1 1 1

grd11020.bgf:
	makebgf -o $*.bgf -r -n "MAR-guild-sideB" -s 2 1 $(SOURCEDIR)\11020w2.BMP 1 1 1

grd11021.bgf:
	makebgf -o $*.bgf -r -n "MAR-rooftiles" -s 2 1 $(SOURCEDIR)\11021f2.BMP 1 1 1

grd11022.bgf:
	makebgf -o $*.bgf -r -n "MAR-aftyn-door" -s 2 1 $(SOURCEDIR)\11022w2.BMP 1 1 1

grd11023.bgf:
	makebgf -o $*.bgf -r -n "MAR-aftyn-front" -s 2 1 $(SOURCEDIR)\11023w2.BMP 1 1 1

grd11024.bgf:
	makebgf -o $*.bgf -r -n "MAR-aftyn-back" -s 2 1 $(SOURCEDIR)\11024w2.BMP 1 1 1

grd11025.bgf:
	makebgf -o $*.bgf -r -n "MAR-raner-door" -s 2 1 $(SOURCEDIR)\11025w2.BMP 1 1 1

grd11026.bgf:
	makebgf -o $*.bgf -r -n "MAR-raner-side" -s 2 1 $(SOURCEDIR)\11026w2.BMP 1 1 1

grd11027.bgf:
	makebgf -o $*.bgf -r -n "MAR-raner-45" -s 2 1 $(SOURCEDIR)\11027w2.BMP 1 1 1

grd11028.bgf:
	makebgf -o $*.bgf -r -n "MAR-barn-door" -s 2 1 $(SOURCEDIR)\11028w2.BMP 1 1 1

grd11029.bgf:
	makebgf -o $*.bgf -r -n "MAR-barn-front" -s 2 1 $(SOURCEDIR)\11029w2.BMP 1 1 1

grd11030.bgf:
	makebgf -o $*.bgf -r -n "MAR-barn-side" -s 2 1 $(SOURCEDIR)\11030w2.BMP 1 1 1

grd11031.bgf:
	makebgf -o $*.bgf -r -n "MAR-barn-back" -s 2 1 $(SOURCEDIR)\11031w2.BMP 1 1 1

grd11032.bgf:
	makebgf -o $*.bgf -r -n "MAR-dirt road" -s 2 1 $(SOURCEDIR)\11032f2.BMP 1 1 1

grd11033.bgf:
	makebgf -o $*.bgf -r -n "MAR-stonewall top" -s 2 1 $(SOURCEDIR)\11033f2.BMP 1 1 1

grd11034.bgf:
	makebgf -o $*.bgf -r -n "MAR-cropA" -s 2 1 $(SOURCEDIR)\11034f2.BMP 1 1 1

grd11035.bgf:
	makebgf -o $*.bgf -r -n "MAR-cropC" -s 2 1 $(SOURCEDIR)\11035f2.BMP 1 1 1

grd11036.bgf:
	makebgf -o $*.bgf -r -n "MAR-flowers" -s 2 1 $(SOURCEDIR)\11036f2.BMP 1 1 1

grd11037.bgf:
	makebgf -o $*.bgf -r -n "MAR-dryleaves" -s 2 1 $(SOURCEDIR)\11037f2.BMP 1 1 1

grd11038.bgf:
	makebgf -o $*.bgf -r -n "MAR-woodtile" -s 2 1 $(SOURCEDIR)\11038w2.BMP 1 1 1

grd11039.bgf:
	makebgf -o $*.bgf -r -n "MAR-stonewall" -s 2 1 $(SOURCEDIR)\11039w2.BMP 1 1 1

grd11040.bgf:
	makebgf -o $*.bgf -r -n "MAR-stonewallB" -s 2 1 $(SOURCEDIR)\11040w2.BMP 1 1 1

grd11041.bgf:
	makebgf -o $*.bgf -r -n "MAR-grass side" -s 2 1 $(SOURCEDIR)\11041w2.BMP 1 1 1

grd11042.bgf:
	makebgf -o $*.bgf -r -n "MAR-wellpole" -s 2 1 $(SOURCEDIR)\11042w2.BMP 1 1 1

grd11043.bgf:
	makebgf -o $*.bgf -r -n "MAR-dirt-to-rock" -s 2 1 $(SOURCEDIR)\11043f2.BMP 1 1 1

grd11050.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-gray cobblewall" -s 2 1 $(SOURCEDIR)\11050w2.BMP 1 1 1

grd11051.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Apoth door" -s 2 1 $(SOURCEDIR)\11051w2.BMP 1 1 1

grd11052.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Apoth edgewall" -s 2 1 $(SOURCEDIR)\11052w2.BMP 1 1 1

grd11053.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Apoth window" -s 2 1 $(SOURCEDIR)\11053w2.BMP 1 1 1

grd11054.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-coblefloor" -s 2 1 $(SOURCEDIR)\11054f2.BMP 1 1 1

grd11055.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casinowall/pilar" -s 2 1 $(SOURCEDIR)\11055w2.BMP 1 1 1

grd11056.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casinowindow" -s 2 1 $(SOURCEDIR)\11056w2.BMP 1 1 1

grd11057.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casinosign" -s 2 1 $(SOURCEDIR)\11057w2.BMP 1 1 1

grd11058.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casinodoor" -s 2 1 $(SOURCEDIR)\11058w2.BMP 1 1 1

grd11059.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casinotopwall" -s 2 1 $(SOURCEDIR)\11059w2.BMP 1 1 1

grd11060.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casino up.window" -s 2 1 $(SOURCEDIR)\11060w2.BMP 1 1 1

grd11061.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casino wall-alt" -s 2 1 $(SOURCEDIR)\11061w2.BMP 1 1 1

grd11062.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-casino door-alt" -s 2 1 $(SOURCEDIR)\11062w2.BMP 1 1 1

grd11063.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn wall" -s 2 1 $(SOURCEDIR)\11063w2.BMP 1 1 1

grd11064.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn wall edge" -s 2 1 $(SOURCEDIR)\11064w2.BMP 1 1 1

grd11065.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn door" -s 2 1 $(SOURCEDIR)\11065w2.BMP 1 1 1

grd11066.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn front window" -s 2 1 $(SOURCEDIR)\11066w2.BMP 1 1 1

grd11067.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn side window" -s 2 1 $(SOURCEDIR)\11067w2.BMP 1 1 1

grd11068.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn side wall" -s 2 1 $(SOURCEDIR)\11068w2.BMP 1 1 1

grd11069.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn upstairs" -s 2 1 $(SOURCEDIR)\11069w2.BMP 1 1 1

grd11070.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-dark roof tiles" -s 2 1 $(SOURCEDIR)\11070f2.BMP 1 1 1

grd11071.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-inn upstairs side" -s 2 1 $(SOURCEDIR)\11071w2.BMP 1 1 1

grd11072.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-adv.Hall facade" -s 2 1 $(SOURCEDIR)\11072w2.BMP 1 1 1

grd11073.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-adv.Hall side" -s 2 1 $(SOURCEDIR)\11073w2.BMP 1 1 1

grd11074.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-adv.Hall steptile" -s 2 1 $(SOURCEDIR)\11074f2.BMP 1 1 1

grd11075.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-lib front wall" -s 2 1 $(SOURCEDIR)\11075w2.BMP 1 1 1

grd11076.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-lib door" -s 2 1 $(SOURCEDIR)\11076w2.BMP 1 1 1

grd11077.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-lib side/window" -s 2 1 $(SOURCEDIR)\11077w2.BMP 1 1 1

grd11078.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-lib sidewallB" -s 2 1 $(SOURCEDIR)\11078w2.BMP 1 1 1

grd11079.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-lib sidewallC" -s 2 1 $(SOURCEDIR)\11079w2.BMP 1 1 1

grd11080.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-topiary side" -s 2 1 $(SOURCEDIR)\11080w2.BMP 1 1 1

grd11081.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-topiary top" -s 2 1 $(SOURCEDIR)\11081f2.BMP 1 1 1

grd11082.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-marble fountain side" -s 2 1 $(SOURCEDIR)\11082w2.BMP 1 1 1

grd11083.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-marble fountain top" -s 2 1 $(SOURCEDIR)\11083f2.BMP 1 1 1

grd11084.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav frontleft" -s 2 1 $(SOURCEDIR)\11084w2.BMP 1 1 1

grd11085.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav frontright" -s 2 1 $(SOURCEDIR)\11085w2.BMP 1 1 1

grd11086.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav side A" -s 2 1 $(SOURCEDIR)\11086w2.BMP 1 1 1

grd11087.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav side B (door)" -s 2 1 $(SOURCEDIR)\11087w2.BMP 1 1 1

grd11088.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tave side C (stairs)" -s 2 1 $(SOURCEDIR)\11088w2.BMP 1 1 1

grd11089.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav rooftiles" -s 2 1 $(SOURCEDIR)\11089f2.BMP 1 1 1

grd11090.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav windowfront" -s 2 1 $(SOURCEDIR)\11090w2.BMP 1 1 1

grd11091.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-tav windowside" -s 2 1 $(SOURCEDIR)\11091w2.BMP 1 1 1

grd11092.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-jail door" -s 2 1 $(SOURCEDIR)\11092w2.BMP 1 1 1

grd11093.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-jail wall A" -s 2 1 $(SOURCEDIR)\11093w2.BMP 1 1 1

grd11094.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-jail slope" -s 2 1 $(SOURCEDIR)\11094f2.BMP 1 1 1

grd11095.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-jail wall end" -s 2 1 $(SOURCEDIR)\11095w2.BMP 1 1 1

grd11096.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar wallA" -s 2 1 $(SOURCEDIR)\11096w2.BMP 1 1 1

grd11097.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar wallB end" -s 2 1 $(SOURCEDIR)\11097w2.BMP 1 1 1

grd11098.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar wallC short" -s 2 1 $(SOURCEDIR)\11098w2.BMP 1 1 1

grd11099.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar wall top only" -s 2 1 $(SOURCEDIR)\11099w2.BMP 1 1 1

grd11100.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar door" -s 2 1 $(SOURCEDIR)\11100w2.BMP 1 1 1

grd11101.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar alcove" -s 2 1 $(SOURCEDIR)\11101w2.BMP 1 1 1

grd11102.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-justicar rooftiles" -s 2 1 $(SOURCEDIR)\11102f2.BMP 1 1 1

grd11103.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-sewer shackside" -s 2 1 $(SOURCEDIR)\11103w2.BMP 1 1 1

grd11104.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-sewer shack door" -s 2 1 $(SOURCEDIR)\11104w2.BMP 1 1 1

grd11105.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-sewer wallA" -s 2 1 $(SOURCEDIR)\11105w2.BMP 1 1 1

grd11106.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-sewer wallB (window)" -s 2 1 $(SOURCEDIR)\11106w2.BMP 1 1 1

grd11107.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-sewer wallC (end)" -s 2 1 $(SOURCEDIR)\11107w2.BMP 1 1 1

grd11108.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Exp.Hall Rear facade" -s 2 1 $(SOURCEDIR)\11108w2.BMP 1 1 1

grd11109.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Exp.Hall wall side" -s 2 1 $(SOURCEDIR)\11109w2.BMP 1 1 1

grd11110.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Exp.Hall wall corner" -s 2 1 $(SOURCEDIR)\11110w2.BMP 1 1 1

grd11111.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Inner city gate" -s 2 1 $(SOURCEDIR)\11111w2.BMP 1 1 1

grd11112.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Wall w/ Drapery" -s 2 1 $(SOURCEDIR)\11112w2.BMP 1 1 1

grd11113.bgf:
	makebgf -o $*.bgf -r -n "BAR.N-Wall w/ Drapery, end" -s 2 1 $(SOURCEDIR)\11113w2.BMP 1 1 1

grd11125.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Rook wallA" -s 2 1 $(SOURCEDIR)\11125w2.BMP 1 1 1

grd11126.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Rook door" -s 2 1 $(SOURCEDIR)\11126w2.BMP 1 1 1

grd11127.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Rook wallB" -s 2 1 $(SOURCEDIR)\11127w2.BMP 1 1 1

grd11128.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB2 wall" -s 2 1 $(SOURCEDIR)\11128w2.BMP 1 1 1

grd11129.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB2 window" -s 2 1 $(SOURCEDIR)\11129w2.BMP 1 1 1

grd11130.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB2 door" -s 2 1 $(SOURCEDIR)\11130w2.BMP 1 1 1

grd11131.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB3 sidewall" -s 2 1 $(SOURCEDIR)\11131w2.BMP 1 1 1

grd11132.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB3 wall end" -s 2 1 $(SOURCEDIR)\11132w2.BMP 1 1 1

grd11133.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB3 window" -s 2 1 $(SOURCEDIR)\11133w2.BMP 1 1 1

grd11134.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB3 door" -s 2 1 $(SOURCEDIR)\11134w2.BMP 1 1 1

grd11135.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB4 topwall only" -s 2 1 $(SOURCEDIR)\11135w2.BMP 1 1 1

grd11136.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Smith vented roof" -s 2 1 $(SOURCEDIR)\11136f2.BMP 1 1 1

grd11137.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Smith iron roof" -s 2 1 $(SOURCEDIR)\11137f2.BMP 1 1 1

grd11138.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-smith door" -s 2 1 $(SOURCEDIR)\11138w2.BMP 1 1 1

grd11139.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-smith wall w/ tapestry" -s 2 1 $(SOURCEDIR)\11139w2.BMP 1 1 1

grd11140.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Smith barewall" -s 2 1 $(SOURCEDIR)\11140w2.BMP 1 1 1

grd11141.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Smith wall edge" -s 2 1 $(SOURCEDIR)\11141w2.BMP 1 1 1

grd11142.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Smith chimeny vent" -s 2 1 $(SOURCEDIR)\11142w2.BMP 1 1 1

grd11143.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Smith wall 45" -s 2 1 $(SOURCEDIR)\11143w2.BMP 1 1 1

grd11144.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB6 wallA" -s 2 1 $(SOURCEDIR)\11144w2.BMP 1 1 1

grd11145.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB6 wall w/ edge" -s 2 1 $(SOURCEDIR)\11145w2.BMP 1 1 1

grd11146.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB6 door" -s 2 1 $(SOURCEDIR)\11146w2.BMP 1 1 1

grd11147.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB6 window/portal, narrow" -s 2 1 $(SOURCEDIR)\11147w2.BMP 1 1 1

grd11148.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB8 corner portal" -s 2 1 $(SOURCEDIR)\11148w2.BMP 1 1 1

grd11149.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB17 wall w/edge" -s 2 1 $(SOURCEDIR)\11149w2.BMP 1 1 1

grd11150.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB17 window" -s 2 1 $(SOURCEDIR)\11150w2.BMP 1 1 1

grd11151.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB17 window, midwall" -s 2 1 $(SOURCEDIR)\11151w2.BMP 1 1 1

grd11152.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB17 wall 45" -s 2 1 $(SOURCEDIR)\11152w2.BMP 1 1 1

grd11153.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB17 door" -s 2 1 $(SOURCEDIR)\11153w2.BMP 1 1 1

grd11154.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB16 door" -s 2 1 $(SOURCEDIR)\11154w2.BMP 1 1 1

grd11155.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB16 wall, narrow w/edge" -s 2 1 $(SOURCEDIR)\11155w2.BMP 1 1 1

grd11156.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB16 window w/o portal" -s 2 1 $(SOURCEDIR)\11156w2.BMP 1 1 1

grd11157.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB16 columns w/ portal" -s 2 1 $(SOURCEDIR)\11157w2.BMP 1 1 1

grd11158.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB9 window" -s 2 1 $(SOURCEDIR)\11158w2.BMP 1 1 1

grd11159.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB9 door" -s 2 1 $(SOURCEDIR)\11159w2.BMP 1 1 1

grd11160.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB9 wall" -s 2 1 $(SOURCEDIR)\11160w2.BMP 1 1 1

grd11161.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Guild facade" -s 2 1 $(SOURCEDIR)\11161w2.BMP 1 1 1

grd11162.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Guild step carving" -s 2 1 $(SOURCEDIR)\11162w2.BMP 1 1 1

grd11163.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Guild steptile" -s 2 1 $(SOURCEDIR)\11163w2.BMP 1 1 1

grd11164.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Expl. Hall facade" -s 2 1 $(SOURCEDIR)\11164w2.BMP 1 1 1

grd11165.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Garden wall/brick" -s 2 1 $(SOURCEDIR)\11165w2.BMP 1 1 1

grd11166.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-Garden wall" -s 2 1 $(SOURCEDIR)\11166w2.BMP 1 1 1

grd11167.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-jeweler side" -s 2 1 $(SOURCEDIR)\11167w2.BMP 1 1 1

grd11168.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-jeweler window" -s 2 1 $(SOURCEDIR)\11168w2.BMP 1 1 1

grd11169.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-jeweler door" -s 2 1 $(SOURCEDIR)\11169w2.BMP 1 1 1

grd11170.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-genehall side" -s 2 1 $(SOURCEDIR)\11170w2.BMP 1 1 1

grd11171.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-genehall door" -s 2 1 $(SOURCEDIR)\11171w2.BMP 1 1 1

grd11172.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-genehall windows" -s 2 1 $(SOURCEDIR)\11172w2.BMP 1 1 1

grd11173.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-genehall wall" -s 2 1 $(SOURCEDIR)\11173w2.BMP 1 1 1

grd11174.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB10 sidewall" -s 2 1 $(SOURCEDIR)\11174w2.BMP 1 1 1

grd11175.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB10 facade" -s 2 1 $(SOURCEDIR)\11175w2.BMP 1 1 1

grd11176.bgf:
	makebgf -o $*.bgf -r -n "BAR.S-SB10 door" -s 2 1 $(SOURCEDIR)\11176w2.BMP 1 1 1

grd50002.bgf:
	makebgf -o $*.bgf -r -n "Shrine Floor1" -s 2 1 $(SOURCEDIR)\50002f2.BMP 1 1 1

grd50003.bgf:
	makebgf -o $*.bgf -r -n "Shrine Wall" -s 2 1 $(SOURCEDIR)\50003w2.BMP 1 1 1

grd50004.bgf:
	makebgf -o $*.bgf -r -n "Shrine Floor3" -s 2 1 $(SOURCEDIR)\50004f2.BMP 1 1 1

grd50005.bgf:
	makebgf -o $*.bgf -r -n "Shrine Floor4" -s 2 1 $(SOURCEDIR)\50005f2.BMP 1 1 1

grd50050.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - carpet A" -s 2 1 $(SOURCEDIR)\50050f2.BMP 1 1 1

grd50051.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - carpet B" -s 2 1 $(SOURCEDIR)\50051f2.BMP 1 1 1

grd50052.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - carpet C" -s 2 1 $(SOURCEDIR)\50052f2.BMP 1 1 1

grd50053.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper A" -s 2 1 $(SOURCEDIR)\50053w2.BMP 1 1 1

grd50054.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper A, picture 1" -s 2 1 $(SOURCEDIR)\50054w2.BMP 1 1 1

grd50055.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper A, picture 2" -s 2 1 $(SOURCEDIR)\50055w2.BMP 1 1 1

grd50056.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper A, picture 3" -s 2 1 $(SOURCEDIR)\50056w2.BMP 1 1 1

grd50057.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper B" -s 2 1 $(SOURCEDIR)\50057w2.BMP 1 1 1

grd50058.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper B, picture 1" -s 2 1 $(SOURCEDIR)\50058w2.BMP 1 1 1

grd50059.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper B, picture 2" -s 2 1 $(SOURCEDIR)\50059w2.BMP 1 1 1

grd50060.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper B, picture 3" -s 2 1 $(SOURCEDIR)\50060w2.BMP 1 1 1

grd50061.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread A, top" -s 2 1 $(SOURCEDIR)\50061f2.BMP 1 1 1

grd50062.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread A, side" -s 2 1 $(SOURCEDIR)\50062w2.BMP 1 1 1

grd50063.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread B, top" -s 2 1 $(SOURCEDIR)\50063f2.BMP 1 1 1

grd50064.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread B, side" -s 2 1 $(SOURCEDIR)\50064w2.BMP 1 1 1

grd50065.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper C" -s 2 1 $(SOURCEDIR)\50065w2.BMP 1 1 1

grd50066.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper C, picture 1" -s 2 1 $(SOURCEDIR)\50066w2.BMP 1 1 1

grd50067.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper C, picture 2" -s 2 1 $(SOURCEDIR)\50067w2.BMP 1 1 1

grd50068.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper C, picture 3" -s 2 1 $(SOURCEDIR)\50068w2.BMP 1 1 1

grd50069.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper D" -s 2 1 $(SOURCEDIR)\50069w2.BMP 1 1 1

grd50070.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper D, picture 1" -s 2 1 $(SOURCEDIR)\50070w2.BMP 1 1 1

grd50071.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper D, picture 2" -s 2 1 $(SOURCEDIR)\50071w2.BMP 1 1 1

grd50072.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - wallpaper D, picture 3" -s 2 1 $(SOURCEDIR)\50072w2.BMP 1 1 1

grd50073.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread C, top" -s 2 1 $(SOURCEDIR)\50073f2.BMP 1 1 1

grd50074.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread C, side" -s 2 1 $(SOURCEDIR)\50074w2.BMP 1 1 1

grd50075.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread D, top" -s 2 1 $(SOURCEDIR)\50075f2.BMP 1 1 1

grd50076.bgf:
	makebgf -o $*.bgf -r -n "Rentable rooms - bedspread D, side" -s 2 1 $(SOURCEDIR)\50076w2.BMP 1 1 1

grd50077.bgf:
	 makebgf -o $*.bgf $(WALL) -n "Mausoleum Brick Dark" -s 1 1 $(SOURCEDIR)\mausbr2.bmp 1 1 1

clean: 
	$(RM) $(OUTDIR)\*

squeakyclean: clean

