// flags: -P -2 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: MLIST.C

#include "..\GLIB\PROCESS.H"
#include "CFILE.H"
#include "KERNEL.H"
#include "MLIST.H"
#include "ERROR.H"

// Builds the list from DEP_END-terminated words. With an owner, the list is
// what it waits on: each pid gets linked both ways.
int List::constructList(unsigned short owner, ...)
{
	unsigned i;
	unsigned n;
	PidEntry entry;
	unsigned *args;
	Process *member;
	Process *ownerProcess;

	asm {
		lea bx, args
		mov ax, bp
		add ax, 12
		mov ss:[bx], ax
		mov ss:[bx+2], ss
		push ss
		pop es
		mov di, ax
		mov ax, DEP_END
		mov dx, 0x400
		mov cx, dx
		cld
		repne scasw
		je found
	}
	halt(__FILE__, 89);	// __LINE__
found:
	asm sub dx, cx
	asm mov n, dx
	if (owner)
		ownerProcess = Kernel::getProcess(owner);
	if (data)
		delete data;
	data = new unsigned[n];
	if (data == 0)
		halt(__FILE__, 110);	// __LINE__
	for (i = 0; i < n; i++) {
		entry = *args++;
		data[i] = entry.value;
		if (owner && entry.isPid()) {
			member = Kernel::getProcess(entry.value);
			member->setActiveDep(owner);
			ownerProcess->setPassiveDep(entry.value);
			if (member->ref == 0)
				member->setRef(ownerProcess->ref);
			else if (ownerProcess->ref == 0)
				ownerProcess->setRef(member->ref);
		}
	}
	count = n;
	// Checks the last member added.
	if (owner)
		member->checkDependencies(0);
	return count;
}

void List::clear(void)
{
	if (data == 0)
		halt(__FILE__, 178);	// __LINE__
	delete data;
	count = 0;
	data = 0;
}

List::~List(void)
{
	if (isAllocated())
		clear();
}

int List::addToList(unsigned short value)
{
	unsigned n;
	unsigned i;

	if (isAllocated()) {
		n = count;
		for (i = 0; i < n; i++)
			if (data[i] == value)
				return n;
	}
	return appendToList(value);
}

int List::appendToList(unsigned short value)
{
	unsigned *newData;
	unsigned i;
	unsigned n;

	if (isAllocated()) {
		n = count;
		// A list of linear processes holds one process of its type.
		if (linearType) {
			ProcessType type;
			PidEntry entry(value);
			PidEntry other;
			Process *p;

			if (entry.isPid()) {
				p = Kernel::getProcess(entry.getPid());
				type = p->processType;
				if (linearType == type)
					for (i = 0; i < n; i++) {
						other = data[i];
						if (other.isPid()) {
							Process *q = Kernel::getProcess(other.getPid());
							if (q->processType == linearType)
								halt(__FILE__, 246);	// __LINE__
						}
					}
			}
		}
		newData = new unsigned[n + 1];
		if (!newData)
			halt(__FILE__, 258);	// __LINE__
		n--;
		for (i = 0; i < n; i++)
			newData[i] = data[i];
		newData[i] = value;
		i++;
		newData[i] = DEP_END;
		delete data;
		data = newData;
		count = n + 2;
		return count;
	}
	return constructList(0, value, DEP_END);
}

// Returns the index the value had.
int List::removeFromList(unsigned short value)
{
	unsigned n;
	unsigned remaining;
	unsigned i;
	unsigned found;
	unsigned *newData;
	unsigned k;

	if (isAllocated()) {
		n = count;
		for (i = 0, remaining = 0; i < n; i++)
			if (getMember(i) != value)
				remaining++;
		if (remaining == 1) {
			clear();
			return 0;
		}
		newData = new unsigned[remaining];
		if (!newData)
			halt(__FILE__, 322);	// __LINE__
		for (i = 0, k = 0; i < n; i++)
			if (getMember(i) != value)
				newData[k++] = getMember(i);
			else
				found = i;
		delete data;
		data = newData;
		count = remaining;
		return found;
	}
	halt(__FILE__, 345);	// __LINE__
}

unsigned List::getMember(unsigned short index)
{
	if (count <= index)
		halt(__FILE__, 352);	// __LINE__
	return data[index];
}

void List::setMember(unsigned short index, unsigned short value)
{
	if (count <= index)
		halt(__FILE__, 361);	// __LINE__
	data[index] = value;
}

int List::isInList(unsigned short value)
{
	unsigned i;

	if (isAllocated())
		for (i = 0; i < count; i++)
			if (getMember(i) == value)
				return i;
	return 0;
}

PidEntry List::isPidInList(unsigned short pid)
{
	unsigned i;
	PidEntry entry;

	if (isAllocated())
		for (i = 0; i < count; i++) {
			entry = getMember(i);
			if (entry.isPid() && entry.getPid() == pid)
				return entry;
		}
	return PidEntry(0);
}

PidEntry List::isPidInList(unsigned short pid, unsigned short &index)
{
	unsigned i;
	PidEntry entry;

	if (isAllocated())
		for (i = 0; i < count; i++) {
			entry = getMember(i);
			if (entry.isPid() && entry.getPid() == pid) {
				index = i;
				return entry;
			}
		}
	return PidEntry(0);
}

void List::setListSize(unsigned short size)
{
	if (isAllocated())
		halt(__FILE__, 444);	// __LINE__
	data = new unsigned[size];
	count = size;
	if (data == 0)
		halt(__FILE__, 450);	// __LINE__
}

void List::load(BaseFile *file)
{
	unsigned n = count;

	if (isAllocated()) {
		// The saved pointer is stale.
		data = 0;
		setListSize(n);
		file->read((char *)data, n * 2);
	}
}

void List::save(BaseFile *file)
{
	if (isAllocated())
		file->write((char *)data, count * 2);
}
