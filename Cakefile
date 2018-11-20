target1: dependency1 dependency2
	touch target1
	echo "made target1"

target2: dependency2 dependency3
	touch target2
	echo "made target2"

target3: target1 target2
	touch target3
	echo "made target3"

dependency1:
	touch dependency1
	echo "made dependency1"

dependency2:
	touch dependency2
	echo "made dependency2"

dependency3:
	touch dependency3
	echo "made dependency3"
