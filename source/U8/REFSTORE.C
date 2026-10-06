// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: REFSTORE.C

#include <mem.h>
#include <alloc.h>
#include "ITEM.H"
#include "KERNEL.H"
#include "REFSTORE.H"
#include "ERROR.H"

#define HIT_EVENT		((ProcessType)0x20b)
#define GOT_HIT_EVENT	((ProcessType)0x20c)

StorageData::StorageData(RefStorage *s, ProcessType type, unsigned short item, unsigned short o, short f)
{
	Kernel::setIdString(pid, "StorageData");
	setProcessType((ProcessType)0x236);
	setRef(item);
	event = type;
	index = 0xffff;
	storage = s;
	other = o;
	force = f;
	invalidated = 0;
}

void StorageData::process(void)
{
	Item item(ref);

	if (Kernel::getCurrentProcess() == pid)
		halt(__FILE__, 47);	// __LINE__
	if (event == GOT_HIT_EVENT) {
		item.gotHit(other, force);
		return;
	}
	if (event == HIT_EVENT) {
		item.hit(other, force);
		return;
	}
	halt(__FILE__, 56);	// __LINE__
}

StorageData::~StorageData(void)
{
	if (!invalidated)
		storage->invalidate(ref);
}

RefStorage::RefStorage(short n)
{
	size = n;
	count = 0;
	if (n) {
		list = new StorageData *[n];
		if (list == 0)
			halt(__FILE__, 78);	// __LINE__
		memset(list, 0, n * sizeof(StorageData *));
	} else
		list = 0;
}

void RefStorage::set(StorageData *data)
{
	list[count] = data;
	data->setIndex(count++);
}

// Drops the events of an item that is gone.
void RefStorage::invalidate(unsigned short item)
{
	StorageData *data;
	int i;

	for (i = 0; i < size; i++) {
		data = list[i];
		if (data && data->ref == item) {
			list[i] = 0;
			if (!data->isTerminated()) {
				data->invalidated = 1;
				data->pop(0);
			}
		}
	}
	for (i = 0; i < size; i++) {
		data = list[i];
		if (data && data->other == item) {
			list[i] = 0;
			data->invalidated = 1;
			data->pop(0);
		}
	}
}

void RefStorage::execute(void)
{
	int i;
	StorageData *data;

	for (i = 0; i < size; i++) {
		data = list[i];
		if (data) {
			data->process();
			if (list[i]) {
				list[i] = 0;
				data->invalidated = 1;
				data->pop(0);
			}
		}
	}
}

RefStorage::~RefStorage(void)
{
	if (list)
		free(list);
}
