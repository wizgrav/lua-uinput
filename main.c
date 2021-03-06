#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define die(str, args...) do { \
        luaL_error(L,str);\
    } while(0)


typedef struct ui_t {
	int fd;
	struct input_event ev;
}ui_t;

static int gcme(lua_State *L){
	ui_t *V = (ui_t *)lua_touserdata(L,lua_upvalueindex(1));
	if(V->fd != -1) ioctl(V->fd, UI_DEV_DESTROY);
	return 0;
}

static int press(lua_State *L){
	ui_t *V = (ui_t *)lua_touserdata(L,lua_upvalueindex(1));
	if(V->fd == -1) die("setup codes first");
	int code = lua_tointeger(L,1);
	memset(&V->ev, 0, sizeof(struct input_event));
	V->ev.type = EV_KEY;
	V->ev.code = code;
	V->ev.value = 1;
	lua_pushinteger(L,write(V->fd, &V->ev, sizeof(V->ev)));
	memset(&V->ev, 0, sizeof(struct input_event));
	 V->ev.type = EV_SYN;
            V->ev.code = 0;
            V->ev.value = 0;
            lua_pushinteger(L,write(V->fd, &V->ev, sizeof(V->ev)));
	
	return 2;
}

static int release(lua_State *L){
	ui_t *V = (ui_t *)lua_touserdata(L,lua_upvalueindex(1));
	if(V->fd == -1) die("setup codes first");
	int code = lua_tointeger(L,1);
	memset(&V->ev, 0, sizeof(struct input_event));
	V->ev.type = EV_KEY;
	V->ev.code = code;
	V->ev.value = 0;
	lua_pushinteger(L,write(V->fd, &V->ev, sizeof(V->ev)));
	memset(&V->ev, 0, sizeof(struct input_event));
	    V->ev.type = EV_SYN;
            V->ev.code = 0;
            V->ev.value = 0;
            lua_pushinteger(L,write(V->fd, &V->ev, sizeof(V->ev)));
	
	return 2;
}


static int setup(lua_State *L){
	int n = lua_gettop(L); 
	ui_t *V = (ui_t *)lua_touserdata(L,lua_upvalueindex(1));
	if(!n) return 0;
	if(V->fd != -1){
		ioctl(V->fd, UI_DEV_DESTROY);
	}
	struct uinput_user_dev uidev;
    struct input_event     ev;
    int                    dx, dy;
    int                    i;
    V->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(V->fd < 0)
        die("error: open");

    if(ioctl(V->fd, UI_SET_EVBIT, EV_KEY) < 0)
        die("error: ioctl");
if(ioctl(V->fd, UI_SET_EVBIT, EV_SYN) < 0)
        die("error: ioctl");
    
    for(i=1;i<=n;i++){
		if(ioctl(V->fd, UI_SET_KEYBIT, lua_tointeger(L,i)) < 0)
			die("error: ioctl");
	}
	
    memset(&uidev, 0, sizeof(uidev));
snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "lua-uinput");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(V->fd, &uidev, sizeof(uidev)) < 0)
        die("error: write");

    if(ioctl(V->fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");
	
	return 0;
}

int LUA_API luaopen_uinput (lua_State *L) {
	ui_t *V = (ui_t *) lua_newuserdata(L,sizeof(ui_t));
	lua_createtable(L,0,0);
	lua_pushstring(L, "__gc");
	lua_pushvalue(L,-3);\
	lua_pushcclosure(L,&gcme,1);
	lua_settable(L, -3);
	lua_setmetatable(L,-2);
	
	lua_createtable(L,0,0);
	luaL_newmetatable(L,"fake.input");
	lua_pushstring(L, "__metatable");
	lua_pushvalue(L,-3);
	lua_settable(L, -3);
	
	lua_pushstring(L, "press");
	lua_pushvalue(L,-4);\
	lua_pushcclosure(L,&press,1);
	lua_settable(L, -3);
	
	lua_pushstring(L, "setup");
	lua_pushvalue(L,-4);\
	lua_pushcclosure(L,&setup,1);
	lua_settable(L, -3);
	
	lua_pushstring(L, "release");
	lua_pushvalue(L,-4);\
	lua_pushcclosure(L,&release,1);
	lua_settable(L, -3);
	
	V->fd = -1;
	return 1;
}



