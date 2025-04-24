from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys, time, math

# Window dimensions
global_width, global_height = 1024, 768
start_time = None
manager = None
h_spacing = 0.1
v_base = 0.1

# Button definitions (screen coords)
BTN_DNS = (200, 250, 350, 320)
BTN_HTTP = (550, 250, 700, 320)
BTN_BACK = (20, 20, 120, 60)

# --- DNS Data ---
dns_nodes = {
    "Resolver": (0, -2),
    "Root": (-3*h_spacing, v_base),
    ".com": (-2*h_spacing, v_base),
    "google.com": (-1*h_spacing, v_base),
    "latest.google.com": (0*h_spacing, v_base),
    "old.latest.google.com": (1*h_spacing, v_base),
    "new.old.latest.google.com": (2*h_spacing, v_base),
    "IP:192.100.2.2": (3*h_spacing, v_base),
}
dns_steps = [
    ["Resolver","Root"],
    ["Resolver",".com"],
    ["Resolver","google.com"],
    ["Resolver","latest.google.com"],
    ["Resolver","old.latest.google.com"],
    ["Resolver","new.old.latest.google.com"],
    ["Resolver","IP:192.100.2.2"],
]

# --- HTTP Data ---
http_nodes = {
    "Client": (-4 * h_spacing, 0),
    "LB": (-2 * h_spacing, 0),
    "Server": (0 * h_spacing, 0),
    "App": (2 * h_spacing, 0),
    "DB": (4 * h_spacing, 0),
}
http_phases = [
    ("Client","LB", "Phase 1: GET Request"),
    ("Server","DB", "Phase 2: Process"),
    ("LB","Client", "Phase 3: Response"),
]

# Helper: draw text in world coords
def draw_text(x, y, text, font=GLUT_BITMAP_HELVETICA_10):
    glRasterPos2f(x, y)
    for ch in text:
        glutBitmapCharacter(font, ord(ch))

# Helper: draw button in screen coords
def draw_button(rect, label):
    x0,y0,x1,y1 = rect
    glColor3f(0.2, 0.2, 0.6)
    glBegin(GL_QUADS)
    glVertex2f(x0,y0); glVertex2f(x1,y0)
    glVertex2f(x1,y1); glVertex2f(x0,y1)
    glEnd()
    glColor3f(1,1,1)
    glRasterPos2f(x0+10, y0+20)
    for ch in label:
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ord(ch))

# Helper: draw arrow with arrowhead (smaller size)
def draw_arrow(p1, p2, color=(0.8,0.8,1)):
    glColor3f(*color)
    glBegin(GL_LINES)
    glVertex2f(*p1); glVertex2f(*p2)
    glEnd()
    dx, dy = p2[0]-p1[0], p2[1]-p1[1]
    angle = math.atan2(dy, dx)
    size = 0.15  # reduced arrowhead size
    for sign in (1, -1):
        a = angle + sign*0.4
        x = p2[0] - size*math.cos(a)
        y = p2[1] - size*math.sin(a)
        glBegin(GL_LINES)
        glVertex2f(*p2); glVertex2f(x,y)
        glEnd()

# --- Scene Base ---
class Scene:
    def update(self, dt): pass
    def render(self): pass
    def mouse(self, x, y): pass

# --- Home Scene ---
class HomeScene(Scene):
    def render(self):
        glClear(GL_COLOR_BUFFER_BIT)
        glLoadIdentity()
        glColor3f(1,1,0)
        draw_text(-2.5, 1.5, "Home: Choose Visualization")
        # Draw DNS and HTTP buttons
        glMatrixMode(GL_PROJECTION)
        glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, global_width, 0, global_height)
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity()
        draw_button(BTN_DNS, "DNS Resolver")
        draw_button(BTN_HTTP, "HTTP Flow")
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW)
        glutSwapBuffers()

    def mouse(self, x, y):
        iy = global_height - y
        if BTN_DNS[0]<x<BTN_DNS[2] and BTN_DNS[1]<iy<BTN_DNS[3]:
            manager.go_to(1)
        if BTN_HTTP[0]<x<BTN_HTTP[2] and BTN_HTTP[1]<iy<BTN_HTTP[3]:
            manager.go_to(2)

# --- DNS Scene ---
class DNSScene(Scene):
    def update(self, dt): self.dt = dt
    def render(self):
        glClear(GL_COLOR_BUFFER_BIT)
        glLoadIdentity()
        draw_dns_panel(self.dt)
        # Draw Back button
        glMatrixMode(GL_PROJECTION)
        glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, global_width, 0, global_height)
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity()
        draw_button(BTN_BACK, "Back")
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW)
        glutSwapBuffers()

    def mouse(self, x, y):
        iy = global_height - y
        if BTN_BACK[0]<x<BTN_BACK[2] and BTN_BACK[1]<iy<BTN_BACK[3]:
            manager.go_to(0)


def draw_dns_panel(dt):
    cycle=7.0
    t=((dt)%cycle)/cycle*len(dns_steps)
    idx=int(t)%len(dns_steps)
    frac=t-idx
    step=dns_steps[idx]
    p1,p2=dns_nodes[step[0]], dns_nodes[step[1]]
    glColor3f(1,1,0); draw_text(-6,3.5,"DNS Resolver")
    for name,(x,y) in dns_nodes.items():
        glColor3f(1,0.8,0) if name in step else glColor3f(0.2,0.8,0.2)
        glPushMatrix(); glTranslatef(x,y,0); glutSolidSphere(0.015,24,24); glPopMatrix()  # smaller nodes
        glColor3f(1,1,1); draw_text(x+0.2,y-0.05,name)
    draw_arrow(p1,p2)
    px=p1[0]+(p2[0]-p1[0])*frac; py=p1[1]+(p2[1]-p1[1])*frac
    glColor3f(1,0.2,0.2); glPushMatrix(); glTranslatef(px,py,0); glutSolidSphere(0.012,16,16); glPopMatrix()  # smaller packet

# --- HTTP Scene ---
class HTTPScene(Scene):
    def update(self, dt): self.dt=dt
    def render(self):
        glClear(GL_COLOR_BUFFER_BIT)
        glLoadIdentity()
        draw_http_panel(self.dt)
        # Draw Back button
        glMatrixMode(GL_PROJECTION)
        glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, global_width, 0, global_height)
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity()
        draw_button(BTN_BACK, "Back")
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW)
        glutSwapBuffers()

    def mouse(self, x, y):
        iy=global_height-y
        if BTN_BACK[0]<x<BTN_BACK[2] and BTN_BACK[1]<iy<BTN_BACK[3]: manager.go_to(0)


def draw_http_panel(dt):
    cycle=6.0
    t=((dt)%cycle)/cycle*len(http_phases)
    idx=int(t)%len(http_phases)
    frac=t-idx
    a,b,txt=http_phases[idx]
    p1,p2=http_nodes[a], http_nodes[b]
    glColor3f(1,1,0); draw_text(-6,3.5,txt)
    for name,(x,y) in http_nodes.items():
        glColor3f(0.2,0.5,0.8)
        glPushMatrix(); glTranslatef(x,y,0); glutSolidCube(0.3); glPopMatrix()  # smaller cube
        glColor3f(1,1,1); draw_text(x-0.3,y-0.3,name)
    draw_arrow(p1,p2,(0.5,0.8,1))
    px=p1[0]+(p2[0]-p1[0])*frac; py=p1[1]+(p2[1]-p1[1])*frac
    glColor3f(1,1,0.2); glPushMatrix(); glTranslatef(px,py,0); glutSolidSphere(0.15,16,16); glPopMatrix()  # smaller packet

# --- Scene Manager ---
class SceneManager:
    def __init__(self): self.scenes=[]; self.idx=0
    def add(self,s): self.scenes.append(s)
    def go_to(self,i): self.idx=i
    def update(self,dt): self.scenes[self.idx].update(dt)
    def render(self): self.scenes[self.idx].render()
    def mouse(self,x,y): self.scenes[self.idx].mouse(x,y)

# --- Callbacks ---
def display():
    global start_time
    if start_time is None: start_time=time.time()
    dt=time.time()-start_time
    manager.update(dt)
    manager.render()


def reshape(w,h):
    global global_width, global_height
    global_width, global_height = w,h
    glViewport(0,0,w,h)


def mouse(button,state,x,y):
    if button==GLUT_LEFT_BUTTON and state==GLUT_DOWN:
        manager.mouse(x,y)

# --- Main ---
def main():
    global manager
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB)
    glutInitWindowSize(global_width, global_height)
    glutCreateWindow(b"Multi-Screen DNS & HTTP Visualizer")
    glClearColor(0.1,0.1,0.1,1)
    # init manager and scenes
    manager=SceneManager()
    manager.add(HomeScene())
    manager.add(DNSScene())
    manager.add(HTTPScene())
    # register callbacks
    glutDisplayFunc(display)
    glutIdleFunc(display)
    glutReshapeFunc(reshape)
    glutMouseFunc(mouse)
    glutMainLoop()

if __name__=='__main__': main()
