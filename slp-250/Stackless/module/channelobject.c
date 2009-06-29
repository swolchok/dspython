/******************************************************

  The Channel

 ******************************************************/

#include "Python.h"

#ifdef STACKLESS
#include "core/stackless_impl.h"
#include "channelobject.h"


static void
channel_clear(PyObject *ob)
{
	PyChannelObject *ch = (PyChannelObject *) ob;

	/*
	 * remove all tasklets and hope they will die.
	 * Note that the channel might receive new actions
	 * during tasklet deallocation, so we don't even know
	 * if it will have the same direction. :-/
	 */
	while (ch->balance) {
		int dir = ch->balance > 0 ? 1 : -1;

		ob = (PyObject *) slp_channel_remove(ch, dir);
		Py_DECREF(ob);
	}
}

static void
channel_dealloc(PyObject *ob)
{
	PyChannelObject *ch = (PyChannelObject *) ob;

	if (ch->balance) {
		if (slp_resurrect_and_kill(ob, channel_clear)) {
			/* the beast has grown new references */
			return;
		}
	}
	if (ch->chan_weakreflist != NULL)
		PyObject_ClearWeakRefs((PyObject *)ch);
	ob->ob_type->tp_free(ob);
}

static int
channel_traverse(PyChannelObject *ch, visitproc visit, void *arg)
{
	int err;
	PyTaskletObject *p;

#define VISIT(o) if (o) {if ((err = visit((PyObject *)(o), arg))) return err;}
	for (p = ch->head; p != (PyTaskletObject *) ch; p = p->next) {
		VISIT(p);
	}
#undef VISIT
	return 0;
}

void
slp_channel_insert(PyChannelObject *channel, PyTaskletObject *task, int dir)
{
	SLP_HEADCHAIN_INSERT(PyTaskletObject, channel, task, next, prev);
	channel->balance += dir;
	task->flags.blocked = dir;
}

PyTaskletObject *
slp_channel_remove(PyChannelObject *channel, int dir)
{
	PyTaskletObject *ret = channel->head;

	assert(PyTasklet_Check(ret));

        channel->balance -= dir;
	SLP_HEADCHAIN_REMOVE(ret, next, prev);
	ret->flags.blocked = 0;
        return ret;
};

/* the special case to remove a specific tasklet */

PyTaskletObject *
slp_channel_remove_specific(PyChannelObject *channel, int dir,
			    PyTaskletObject *task)
{
	/* note: we assume that the task is in the channel! */

	assert(PyTasklet_Check(task));
	channel->balance -= dir;
	SLP_HEADCHAIN_REMOVE(task, next, prev);
	task->flags.blocked = 0;
	return task;
}

/* freeing a tasklet without an explicit channel */

PyTaskletObject *
slp_channel_remove_slow(PyTaskletObject *task)
{
	int dir;
	PyChannelObject *channel;
	PyTaskletObject *prev = task->prev;

	assert(task->flags.blocked);
	while (!PyChannel_Check(prev))
		prev = prev->prev;
	channel = (PyChannelObject *) prev;
	assert(channel->balance);
	dir = channel->balance > 0 ? 1 : -1;
	return slp_channel_remove_specific(channel, dir, task);;
}


PyChannelObject *
PyChannel_New(PyTypeObject *type)
{
	PyChannelObject *c;

	if (type == NULL)
		type = &PyChannel_Type;
	if (!PyType_IsSubtype(type, &PyChannel_Type))
		TYPE_ERROR("channel_new: type must be subtype of channel", NULL);
	c = (PyChannelObject *) type->tp_alloc(type, 0);
	if (c != NULL) {
		c->head = c->tail = (PyTaskletObject *) c;
		c->balance = 0;
		c->chan_weakreflist = NULL;
		*(int*)&c->flags = 0;
		c->flags.preference = -1; /* default fast receive */
	}
	return c;
}

static PyObject *
channel_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
        static char *argnames[] = {NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, ":channel", argnames))
		return NULL;
	return (PyObject *)PyChannel_New(type);
}

static PyObject *
channel_get_queue(PyChannelObject *self)
{
	PyObject *ret = (PyObject*) self->head;

	if (ret == (PyObject *) self)
		ret = Py_None;
	Py_INCREF(ret);
	return ret;
}

PyObject *
PyChannel_GetQueue(PyChannelObject *self)
{
	return channel_get_queue(self);
}

static PyObject *
channel_get_closing(PyChannelObject *self)
{
	return PyBool_FromLong(self->flags.closing);
}

int
PyChannel_GetClosing(PyChannelObject *self)
{
	return self->flags.closing;
}

static PyObject *
channel_get_closed(PyChannelObject *self)
{
	return PyBool_FromLong(self->flags.closing && self->balance == 0);
}

int
PyChannel_GetClosed(PyChannelObject *self)
{
	return self->flags.closing && self->balance == 0;
}


static PyObject *
channel_get_preference(PyChannelObject *self)
{
	return PyInt_FromLong(self->flags.preference);
}

static int
channel_set_preference(PyChannelObject *self, PyObject *value)
{
	int val;

	if (!PyInt_Check(value))
		TYPE_ERROR("preference must be set to an integer", -1);
	val = PyInt_AS_LONG(value);
	self->flags.preference = val > 0 ? 1 : val < 0 ? -1 : 0;
	return 0;
}

int
PyChannel_GetPreference(PyChannelObject *self)
{
	return self->flags.preference;
}

void
PyChannel_SetPreference(PyChannelObject *self, int val)
{
	self->flags.preference = val > 0 ? 1 : val < 0 ? -1 : 0;
}

static PyObject *
channel_get_schedule_all(PyChannelObject *self)
{
	return PyInt_FromLong(self->flags.schedule_all);
}

static int
channel_set_schedule_all(PyChannelObject *self, PyObject *value)
{
	if (!PyInt_Check(value))
		TYPE_ERROR("preference must be set to a bool or integer", -1);
	self->flags.schedule_all = PyInt_AsLong(value) ? 1 : 0;
	return 0;
}

int
PyChannel_GetScheduleAll(PyChannelObject *self)
{
	return self->flags.schedule_all;
}

void
PyChannel_SetScheduleAll(PyChannelObject *self, int val)
{
	self->flags.schedule_all = val ? 1 : 0;
}

static PyGetSetDef channel_getsetlist[] = {
	{"queue",		(getter)channel_get_queue, NULL,
	 "the chain of waiting tasklets."},
	{"closing",		(getter)channel_get_closing, NULL,
	 "True when close was called."},
	{"closed",		(getter)channel_get_closed, NULL,
	 "True when close was called and the channel is empty.."},
	{"preference",		(getter)channel_get_preference,
				(setter)channel_set_preference,
	 "-1 prefer receiver (default), 1 prefer sender, 0 don't\n"
	 "prefer anything. See also schedule_all"},
	{"schedule_all",	(getter)channel_get_schedule_all,
				(setter)channel_set_schedule_all,
	 "schedule to the next runnable on any channel action.\n"
	 "overrides preference."},
	{0}
};


static PyMemberDef channel_members[] = {
	{"balance", T_INT, offsetof(PyChannelObject, balance), READONLY,
	 "the number of tasklets waiting to send (>0) or receive (<0)."},
	{0}
};

/**********************************************************

  The central functions of the channel concept.
  A tasklet can either send or receive on a channel.
  A channel has a queue of waiting tasklets.
  They are either all waiting to send or all
  waiting to receive.
  Initially, a channel is in a neutral state.
  The queue is empty, there is no way to
  send or receive without becoming blocked.

  Sending 1):
    A tasklet wants to send and there is
    a queued receiving tasklet. The sender puts
    its data into the receiver, unblocks it,
    and inserts it at the top of the runnables.
    The receiver is scheduled.
  Sending 2):
    A tasklet wants to send and there is
    no queued receiving tasklet.
    The sender will become blocked and inserted
    into the queue. The next receiver will
    handle the rest through "Receiving 1)".
  Receiving 1):
    A tasklet wants to receive and there is
    a queued sending tasklet. The receiver takes
    its data from the sender, unblocks it,
    and inserts it at the end of the runnables.
    The receiver continues with no switch.
  Receiving 2):
    A tasklet wants to receive and there is
    no queued sending tasklet.
    The receiver will become blocked and inserted
    into the queue. The next sender will
    handle the rest through "Sending 1)".
 */


static PyObject * channel_hook = NULL;

static void
channel_callback(PyChannelObject *channel, PyTaskletObject *task, int sending, int willblock)
{
	PyObject *args, *ret;
	PyObject *type, *value, *traceback;
	args = ret = NULL;

	args = Py_BuildValue("(OOii)", channel, task, sending, willblock);
	if (args != NULL) {

		PyErr_Fetch(&type, &value, &traceback);
		ret = PyObject_Call(channel_hook, args, NULL);

		if (ret != NULL) {
			PyErr_Restore(type, value, traceback);
		}
		else {
			Py_XDECREF(type);
			Py_XDECREF(value);
			Py_XDECREF(traceback);
		}

		Py_XDECREF(ret);
		Py_DECREF(args);
	}
}

#define NOTIFY_CHANNEL(channel, task, dir, cando, res) \
    if(channel_hook != NULL) { \
      if (ts->st.schedlock) {  \
	RUNTIME_ERROR("Recursive channel call due to callbacks!", res); \
      } \
      ts->st.schedlock = 1; \
      channel_callback(channel, task, dir > 0, !cando); \
      ts->st.schedlock = 0;\
   }


int PyStackless_SetChannelCallback(PyObject *callable)
{
	if(callable != NULL && !PyCallable_Check(callable))
		TYPE_ERROR("channel callback must be callable", -1);
	Py_XDECREF(channel_hook);
	Py_XINCREF(callable);
	channel_hook = callable;
	return 0;
}


static char channel_send__doc__[] =
"channel.send(value) -- send a value over the channel.\n\
If no other tasklet is already receiving on the channel,\n\
the sender will be blocked. Otherwise, the receiver will\n\
be activated immediately, and the sender is put at the end of\n\
the runnables list.";

static PyObject *
PyChannel_Send_M(PyChannelObject *self, PyObject *arg)
{
	return PyStackless_CallMethod_Main((PyObject *) self, "send", "(O)",
					   arg);
}

int
PyChannel_Send_nr(PyChannelObject *self, PyObject *arg)
{
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;

	slp_try_stackless = slp_enable_softswitch;
	return slp_return_wrapper(t->send(self, arg));
}

int
PyChannel_Send(PyChannelObject *self, PyObject *arg)
{
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;

	return slp_return_wrapper(t->send(self, arg));
}


/*
 * This generic function exchanges values over a channel.
 * the action can be either send or receive.
 * Note that this works even across threads. The insert action
 * uses the tstate which is stored in the target.
 */

static PyObject *
generic_channel_action(PyChannelObject *self, PyObject *arg, int dir, int stackless)
{
	PyThreadState *ts = PyThreadState_GET();
	PyTaskletObject *source = ts->st.current;
	PyTaskletObject *target = self->head;
	int cando = dir > 0 ? self->balance < 0 : self->balance > 0;
	int interthread = cando ? target->cstate->tstate != ts : 0;
	PyObject *retval;

	assert(abs(dir) == 1);

	TASKLET_SETVAL(source, arg);

	/* note that notify might release the GIL. */
	/* XXX for the moment, we notify late on interthread */
	if (!interthread)
		NOTIFY_CHANNEL(self, source, dir, cando, NULL);

	if (cando) {
		/* communication 1): there is somebody waiting */
		target = slp_channel_remove(self, -dir);
		/* exchange data */
		TASKLET_SWAPVAL(source, target);

		if (interthread) {
			/* interthread, always keep target! */
			slp_current_insert(target);
		}
		else {
			if (self->flags.schedule_all) {
				/* target goes last */
				slp_current_insert(target);
				/* always schedule away from source */
				target = source->next;
			}
			else if (self->flags.preference == -dir) {
				/* move target after source */
				ts->st.current = source->next;
				slp_current_insert(target);
				ts->st.current = source;
			}
			else {
				/* otherwise we return to the caller */
				slp_current_insert(target);
				target = source;
			}
		}
	}
	else {
		/* communication 2): there is nobody waiting */
		if (source->flags.block_trap)
			RUNTIME_ERROR("this tasklet does not like to be"
				      " blocked.", NULL);
		if (self->flags.closing) {
			PyErr_SetNone(PyExc_StopIteration);
			return NULL;
		}
		slp_current_remove();
		slp_channel_insert(self, source, dir);
		target = ts->st.current;
	}
	retval = slp_schedule_task(source, target, stackless);
	if (interthread)
		NOTIFY_CHANNEL(self, source, dir, cando, NULL);
	return retval;
}

static CHANNEL_SEND_HEAD(impl_channel_send)
{
	STACKLESS_GETARG();
	PyThreadState *ts = PyThreadState_GET();

	if(ts->st.main == NULL) return PyChannel_Send_M(self, arg);
	return generic_channel_action(self, arg, 1, stackless);
}

static CHANNEL_SEND_HEAD(wrap_channel_send)
{
	return PyObject_CallMethod((PyObject *) self, "send", "(O)", arg);
}

static PyObject *
channel_send(PyObject *myself, PyObject *arg)
{
	return impl_channel_send((PyChannelObject*)myself, arg);
}


static char channel_send_exception__doc__[] =
"channel.send_exception(exc, value) -- send an exception over the channel.\n\
exc must be a subclass of Exception.\n\
Behavior is like channel.send, but that the receiver gets an exception.";

static PyObject *
PyChannel_SendException_M(PyChannelObject *self, PyObject *klass,
			  PyObject *args)
{
	return PyStackless_CallMethod_Main((PyObject *) self,
		"send_exception", "(OO)", klass, args);
}

int
PyChannel_SendException(PyChannelObject *self, PyObject *klass, PyObject *args)
{
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;

	return slp_return_wrapper(t->send_exception(self, klass, args));
}

static CHANNEL_SEND_EXCEPTION_HEAD(impl_channel_send_exception)
{
	STACKLESS_GETARG();
	PyThreadState *ts = PyThreadState_GET();
	PyObject *bomb, *ret = NULL;

	assert(PyChannel_Check(self));
	if (ts->st.main == NULL)
		return PyChannel_SendException_M(self, klass, args);

	bomb = slp_make_bomb(klass, args, "channel.send_exception");
	if (bomb != NULL) {
		ret = generic_channel_action(self, bomb, 1, stackless);
		Py_DECREF(bomb);
	}
	return ret;
}

static CHANNEL_SEND_EXCEPTION_HEAD(wrap_channel_send_exception)
{
	return PyObject_CallMethod((PyObject *) self, "send_exception",
				   "(OO)", klass, args);
}

static PyObject *
channel_send_exception(PyObject *myself, PyObject *args)
{
	STACKLESS_GETARG();
	PyObject *retval = NULL;
	PyObject *klass = PySequence_GetItem(args, 0);

	if (klass == NULL)
		VALUE_ERROR("channel.send_exception(e, v...)", NULL);
	args = PySequence_GetSlice(args, 1, PySequence_Size(args));
	if (!args) {
		goto err_exit;
	}
	STACKLESS_PROMOTE_ALL();
	retval = impl_channel_send_exception((PyChannelObject*)myself,
						klass, args);
	STACKLESS_ASSERT();
	if (retval == NULL || STACKLESS_UNWINDING(retval)) {
		goto err_exit;
	}
	Py_INCREF(Py_None);
	retval = Py_None;
err_exit:
	Py_DECREF(klass);
	Py_XDECREF(args);
	return retval;
}

static char channel_receive__doc__[] =
"channel.receive() -- receive a value over the channel.\n\
If no other tasklet is already sending on the channel,\n\
the receiver will be blocked. Otherwise, the receiver will\n\
continue immediately, and the sender is put at the end of\n\
the runnables list.\n\
The above policy can be changed by setting channel flags.";

static PyObject *
PyChannel_Receive_M(PyChannelObject *self)
{
	return PyStackless_CallMethod_Main((PyObject *) self, "receive", NULL);
}

PyObject *
PyChannel_Receive_nr(PyChannelObject *self)
{
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;
	PyObject *ret;

	slp_try_stackless = 1;
	ret = t->receive(self);
	STACKLESS_ASSERT();
	return ret;
}

PyObject *
PyChannel_Receive(PyChannelObject *self)
{
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;
	return t->receive(self);
}

static CHANNEL_RECEIVE_HEAD(impl_channel_receive)
{
	STACKLESS_GETARG();
	PyThreadState *ts = PyThreadState_GET();

	if (ts->st.main == NULL) return PyChannel_Receive_M(self);
	return generic_channel_action(self, Py_None, -1, stackless);
}

static CHANNEL_RECEIVE_HEAD(wrap_channel_receive)
{
	return PyObject_CallMethod((PyObject *) self, "receive", NULL);
}

static PyObject *
channel_receive(PyObject *self)
{
	return impl_channel_receive((PyChannelObject*)self);
}


/*********************************************************

  Sequences in channels.

  Channels can work as an iterator. This removes all
  call overhead on the receiver side and is optimum.

  There is still the sender who has to call a method to
  transfer data.
  But to my surprize, supporting a sequence protocol
  for sending does exactly the desired job.

  Example 1:

  def sender(ch):
    while condition:
      # compute a bulk of results
      ch.send:sequence(results)

  def receiver(ch):
    for each in ch:
      # process each

  Now, with a little layer, we can even avoid the list
  creation, and use a generator to yield the results
  into the channel. This is also helpful to overcome
  the generator restriction of only one yield level.

  Example 2:

  def sender(ch, src):

    def parser():
      while more_to_parse:
        # find next token
	if is_simple_token:
	  yield token
	else:
	  # got nested structure
	  ch.send_sequence(parser())

    parser()

 *********************************************************/


/*
 * iterator extension.
 * This is probably the fastest way to run through a channel
 * from Python.
 */

static PyObject *
channel_iternext(PyChannelObject *self)
{
	STACKLESS_GETARG();

	if (self->flags.closing && self->balance == 0) {
		/* signal the end of the iteration */
		return NULL;
	}
	STACKLESS_PROMOTE_ALL();
	return impl_channel_receive(self);
}

static PyObject *
channel_getiter(PyObject *self)
{
	Py_INCREF(self);
	return self;
}

/*
 * sequence support for sending.
 * This is now absolutely unbeatable, because you can
 * let a generator yield into the channel, without
 * any extra method call. :-)
 */

static char channel_send_sequence__doc__[] =
"channel.send_sequence(seq) -- sed a stream of values\n\
over the channel. Combined with a generator, this is\n\
a very efficient way to build fast pipes.";

/*
 * this is the traight-forward and simple implementation,
 * but here we have almost no speedup, since all switches
 * are hard.
 */

static PyObject *
_channel_send_sequence(PyChannelObject *self, PyObject *v)
{
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;
	PyObject *it;
	int i;
	PyObject *ret;

	it = PyObject_GetIter(v);
	if (it == NULL)
		return NULL;

	/* Run iterator to exhaustion. */
	for (i = 0; ; i++) {
		PyObject *item = PyIter_Next(it);
		if (item == NULL) {
			if (PyErr_Occurred())
				goto error;
			break;
		}
		ret = t->send(self, item);
		Py_DECREF(item);
		if (ret == NULL)
			goto error;
		Py_DECREF(ret);
	}

	Py_DECREF(it);
	return PyInt_FromLong(i);

  error:
	Py_DECREF(it);
	return NULL;

}

/*
 * I tried to make this worker loop look like the simple
 * implementation. The problem is to leave and enter
 * the loop all the time. Hopefully the idea is still visible.
 */

static PyObject *
channel_seq_callback(PyFrameObject *_f, int exc, PyObject *retval)
{
	PyThreadState *ts;
	PyCFrameObject *f = (PyCFrameObject *) _f;
	PyChannelObject *ch;
	PyChannel_HeapType *t;
	PyObject *item;
	int stage = f->n;

	/* prolog to re-enter the loop */
	if (stage == 1) {
		item = retval;
		goto back_with_data;
	}
	if (retval == NULL)
		goto exit_frame;

	Py_DECREF(retval);
	retval = NULL;

	if (stage == 2) {
		goto back_from_send;
	}

	/* Run iterator to exhaustion. */
	for (; ; f->i++) {
		/* get the data */
		STACKLESS_PROPOSE_ALL();
		item = PyIter_Next(f->ob1);

		if (STACKLESS_UNWINDING(item)) {
			stage = f->n = 1;
			return item;
		}

back_with_data:
		if (item == NULL) {
			if (PyErr_Occurred()) {
				if (PyErr_ExceptionMatches(
					PyExc_StopIteration))
					PyErr_Clear();
				else
					goto exit_frame;
			}
			break;
		}

		/* send the data */
		ch = (PyChannelObject *) f->ob2;
		t = (PyChannel_HeapType *) ch->ob_type;
		STACKLESS_PROPOSE_ALL();
		retval = t->send(ch, item);
		Py_DECREF(item);
		if (retval == NULL)
			goto exit_frame;
		if (STACKLESS_UNWINDING(retval)) {
			stage = f->n = 2;
			return retval;
		}
		Py_DECREF(retval);
back_from_send:
		;
	}

	retval = PyInt_FromLong(f->i);
exit_frame:

	/* epilog to return from the frame */
	ts = PyThreadState_GET();
	ts->frame = f->f_back;
	Py_DECREF(f);
	return retval;
}

static PyObject *
channel_send_sequence(PyChannelObject *self, PyObject *v)
{
	STACKLESS_GETARG();
	PyThreadState *ts = PyThreadState_GET();
	PyChannel_HeapType *t = (PyChannel_HeapType *) self->ob_type;
	PyObject *it;
	PyCFrameObject *f;

	if (!stackless)
		return _channel_send_sequence(self, v);

	it = PyObject_GetIter(v);
	if (it == NULL)
		return NULL;

	f = slp_cframe_new(channel_seq_callback, 1);
	if (f == NULL)
		goto error;

	f->ob1 = it;
	Py_INCREF(self);
	f->ob2 = (PyObject *) self;
	f->i = 0;
	f->n = 0;
	ts->frame = (PyFrameObject *) f;
	Py_INCREF(Py_None);
	return STACKLESS_PACK(Py_None);
error:
	Py_DECREF(it);
	return NULL;
}


static char channel_close__doc__[] =
"channel.close() -- stops the channel from enlarging its queue.\n\
\n\
If the channel is not empty, the flag 'closing' becomes true.\n\
If the channel is empty, the flag 'closed' becomes true.";

static PyObject *
channel_close(PyChannelObject *self)
{
	self->flags.closing = 1;

	Py_INCREF(Py_None);
	return Py_None;
}

static char channel_open__doc__[] =
"channel.open() -- reopen a channel. See channel.close.";

static PyObject *
channel_open(PyChannelObject *self)
{
	self->flags.closing = 0;

	Py_INCREF(Py_None);
	return Py_None;
}

static char channel_reduce__doc__[] =
"channel.__reduce__() -- currently does not distinguish threads.";

static PyObject *
channel_reduce(PyChannelObject * ch)
{
	PyObject *tup = NULL, *lis = NULL;
	PyTaskletObject *t;
	int i, n;

	lis = PyList_New(0);
	if (lis == NULL) goto err_exit;
	t = ch->head;
	n = abs(ch->balance);
	for (i = 0; i < n; i++) {
		if (PyList_Append(lis, (PyObject *) t)) goto err_exit;
		t = t->next;
	}
	tup = Py_BuildValue("(O()(iiO))",
			    ch->ob_type,
			    ch->balance,
			    ch->flags,
			    lis
			    );
err_exit:
	Py_XDECREF(lis);
	return tup;
}

static char channel_setstate__doc__[] =
"channel.__setstate__(balance, flags, [tasklets]) -- currently does not distinguish threads.";

static PyObject *
channel_setstate(PyObject *self, PyObject *args)
{
	PyChannelObject *ch = (PyChannelObject *) self;
	PyTaskletObject *t;
	PyObject *lis;
	int flags, balance;
	int dir;
	int i, n;

	if (!PyArg_ParseTuple(args, "iiO!:channel",
			      &balance,
			      &flags,
			      &PyList_Type, &lis))
		return NULL;

	channel_clear((PyObject *) ch);
	n = PyList_GET_SIZE(lis);
	*(int *)&ch->flags = flags;
	dir = balance > 0 ? 1 : -1;

	for (i = 0; i < n; i++) {
		t = (PyTaskletObject *) PyList_GET_ITEM(lis, i);

		if (PyTasklet_Check(t) && !t->flags.blocked) {
			Py_INCREF(t);
			slp_channel_insert(ch, t, dir);
		}
	}
	Py_INCREF(self);
	return self;
}


static PyCMethodDef
channel_cmethods[] = {
	CMETHOD_PUBLIC_ENTRY(PyChannel_HeapType, channel, send),
	CMETHOD_PUBLIC_ENTRY(PyChannel_HeapType, channel, send_exception),
	CMETHOD_PUBLIC_ENTRY(PyChannel_HeapType, channel, receive),
	{NULL}                       /* sentinel */
};


#define PCF PyCFunction
#define METH_KS METH_KEYWORDS | METH_STACKLESS
#define METH_VS METH_VARARGS | METH_STACKLESS
#define METH_NS METH_NOARGS | METH_STACKLESS
#define METH_OS METH_O | METH_STACKLESS

static PyMethodDef
channel_methods[] = {
	{"send",	    (PCF)channel_send,		    METH_OS,
	 channel_send__doc__},
	{"send_exception",  (PCF)channel_send_exception,    METH_VS,
	 channel_send_exception__doc__},
	{"receive",	    (PCF)channel_receive,	    METH_NS,
	 channel_receive__doc__},
	{"close",	    (PCF)channel_close,		    METH_NOARGS,
	channel_close__doc__},
	{"open",	    (PCF)channel_open,		    METH_NOARGS,
	channel_open__doc__},
	{"__reduce__",		(PCF)channel_reduce,	    METH_NOARGS,
	 channel_reduce__doc__},
	{"__reduce_ex__",	(PCF)channel_reduce,	    METH_VARARGS,
	 channel_reduce__doc__},
	{"__setstate__",	(PCF)channel_setstate,	    METH_O,
	 channel_setstate__doc__},
	{"send_sequence",   (PCF)channel_send_sequence,	  METH_OS,
	 channel_send__doc__},
	{NULL,		    NULL}             /* sentinel */
};

static char channel__doc__[] =
"A channel object is used for communication between tasklets.\n\
By sending on a channel, a tasklet that is waiting to receive\n\
is resumed. If there is no waiting receiver, the sender is suspended.\n\
By receiving from a channel, a tasklet that is waiting to send\n\
is resumed. If there is no waiting sender, the receiver is suspended.\
";

PyTypeObject _PyChannel_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"channel",
	sizeof(PyChannelObject),
	0,
	(destructor)channel_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	PyObject_GenericSetAttr,		/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
		Py_TPFLAGS_BASETYPE,		/* tp_flags */
	channel__doc__,				/* tp_doc */
 	(traverseproc)channel_traverse,		/* tp_traverse */
	(inquiry) channel_clear,		/* tp_clear */
	0,					/* tp_richcompare */
	offsetof(PyChannelObject, chan_weakreflist),
						/* tp_weaklistoffset */
	(getiterfunc)channel_getiter,		/* tp_iter */
	(iternextfunc)channel_iternext,		/* tp_iternext */
	channel_methods,			/* tp_methods */
	channel_members,			/* tp_members */
	channel_getsetlist,			/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	channel_new,				/* tp_new */
	_PyObject_GC_Del,			/* tp_free */
};

STACKLESS_DECLARE_METHOD(PyChannel_TypePtr, tp_iternext)

PyTypeObject *PyChannel_TypePtr = &_PyChannel_Type;

/******************************************************

  source module initialization

 ******************************************************/

int init_channeltype(void)
{
	PyTypeObject *t = &_PyChannel_Type;

	if ( (t = PyFlexType_Build("stackless", "channel", t->tp_doc,
				   t, sizeof(PyChannel_HeapType),
				   channel_cmethods) ) == NULL)
		return -1;
	PyChannel_TypePtr = t;
	return 0;
}
#endif
