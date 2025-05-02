from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys, time, math

# Window dimensions
global_width, global_height = 1024, 768
start_time = None
manager = None
h_spacing = 0.4  # Increased horizontal spacing
v_base = 0.5     # Increased vertical base
v_spacing = 0.6  # Vertical spacing
dns_mode = "recursive"  # Default DNS query mode

# Button definitions (screen coords)
BTN_DNS = (350, 350, 650, 420)
BTN_HTTP = (350, 250, 650, 320)
BTN_BACK = (20, 20, 120, 60)
BTN_RECURSIVE = (200, 700, 400, 740)
BTN_ITERATIVE = (600, 700, 800, 740)

# --- DNS Data - Recursive Mode ---
dns_nodes_recursive = {
    "Your Computer": (0, -2.0),
    "Local DNS Resolver": (0, -0.5),
    "Root Server": (-3.0, 1.0),
    ".com TLD Server": (-1.5, 1.0),
    "google.com NS": (0, 1.0),
    "www.google.com": (1.5, 1.0),
    "IP: 142.250.190.36": (3.0, 1.0),
}

dns_steps_recursive = [
    ["Your Computer", "Local DNS Resolver", "Step 1: Request for www.google.com", "User sends lookup request to local resolver"],
    ["Local DNS Resolver", "Root Server", "Step 2: Root Server Query", "Resolver asks root server: Where is .com?"],
    ["Root Server", "Local DNS Resolver", "Step 3: TLD Server Referral", "Root returns referral to .com TLD servers"],
    ["Local DNS Resolver", ".com TLD Server", "Step 4: TLD Server Query", "Resolver asks TLD: Where is google.com?"],
    [".com TLD Server", "Local DNS Resolver", "Step 5: Authoritative NS Referral", "TLD returns referral to google.com name servers"],
    ["Local DNS Resolver", "google.com NS", "Step 6: Authoritative Server Query", "Resolver asks: What's the IP for www.google.com?"],
    ["google.com NS", "Local DNS Resolver", "Step 7: IP Address Response", "Name server returns IP address 142.250.190.36"],
    ["Local DNS Resolver", "Your Computer", "Step 8: Final Response", "Resolver returns the IP to your computer"],
]

# --- DNS Data - Iterative Mode ---
dns_nodes_iterative = {
    "Your Computer": (0, -2.0),
    "Root Server": (-3.0, 1.0),
    ".com TLD Server": (-1.5, 1.0),
    "google.com NS": (0, 1.0), 
    "www.google.com": (1.5, 1.0),
    "IP: 142.250.190.36": (3.0, 1.0),
}

dns_steps_iterative = [
    ["Your Computer", "Root Server", "Step 1: Root Server Query", "User asks root server: Where is www.google.com?"],
    ["Root Server", "Your Computer", "Step 2: TLD Server Referral", "Root server: I don't know, ask .com TLD servers"],
    ["Your Computer", ".com TLD Server", "Step 3: TLD Server Query", "User asks .com TLD: Where is www.google.com?"],
    [".com TLD Server", "Your Computer", "Step 4: Authoritative NS Referral", "TLD: I don't know, ask google.com name servers"],
    ["Your Computer", "google.com NS", "Step 5: Authoritative Server Query", "User asks: What's the IP for www.google.com?"],
    ["google.com NS", "Your Computer", "Step 6: IP Address Response", "Name server returns IP address 142.250.190.36"],
]

# Caption definitions
captions = {
    "recursive": "Recursive DNS: Local resolver does all the work on behalf of client",
    "iterative": "Iterative DNS: Client does all the work, contacting each server in sequence"
}

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

def draw_text_large(x, y, text):
    glRasterPos2f(x, y)
    for ch in text:
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ord(ch))

# Helper: draw button in screen coords
def draw_button(rect, label):
    x0,y0,x1,y1 = rect
    glColor3f(0.3, 0.4, 0.8)  # Nicer blue color
    glBegin(GL_QUADS)
    glVertex2f(x0,y0); glVertex2f(x1,y0)
    glVertex2f(x1,y1); glVertex2f(x0,y1)
    glEnd()
    
    # Button border
    glColor3f(0.4, 0.5, 0.9)
    glLineWidth(2.0)
    glBegin(GL_LINE_LOOP)
    glVertex2f(x0,y0); glVertex2f(x1,y0)
    glVertex2f(x1,y1); glVertex2f(x0,y1)
    glEnd()
    
    glColor3f(1,1,1)
    glRasterPos2f(x0+20, y0+30)  # Better text positioning
    for ch in label:
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ord(ch))

# Draw a nicer node
def draw_node(x, y, radius, color):
    glColor3f(*color)
    
    # Draw main circle
    glPushMatrix()
    glTranslatef(x, y, 0)
    glutSolidSphere(radius, 24, 24)
    glPopMatrix()
    
    # Draw highlight effect
    glColor3f(color[0]+0.2, color[1]+0.2, color[2]+0.2)
    glPushMatrix()
    glTranslatef(x+radius*0.3, y+radius*0.3, 0)
    glutSolidSphere(radius*0.3, 16, 16)
    glPopMatrix()

# Helper: draw arrow with arrowhead (smaller size)
def draw_arrow(p1, p2, color=(0.8, 0.8, 1), width=1.0):
    glColor3f(*color)
    glLineWidth(width)
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
        glColor3f(1,1,0.5)  # Warmer yellow
        draw_text_large(-2, 1.5, "Network Visualization")
        draw_text(-2, 1.2, "Select a visualization type below")
        
        # Draw DNS and HTTP buttons
        glMatrixMode(GL_PROJECTION)
        glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, global_width, 0, global_height)
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity()
        draw_button(BTN_DNS, "DNS Resolution Process")
        draw_button(BTN_HTTP, "HTTP Request Flow")
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
        
        # Draw mode selection buttons
        draw_button(BTN_RECURSIVE, "Recursive Mode")
        draw_button(BTN_ITERATIVE, "Iterative Mode")
        
        # Draw mode indicator
        glColor3f(1, 1, 1)
        glRasterPos2f(450, 675)
        text = "Current Mode: " + dns_mode.capitalize()
        for ch in text:
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ord(ch))
            
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW)
        glutSwapBuffers()

    def mouse(self, x, y):
        iy = global_height - y
        if BTN_BACK[0]<x<BTN_BACK[2] and BTN_BACK[1]<iy<BTN_BACK[3]:
            manager.go_to(0)
        if BTN_RECURSIVE[0]<x<BTN_RECURSIVE[2] and BTN_RECURSIVE[1]<iy<BTN_RECURSIVE[3]:
            switch_dns_mode("recursive")
        if BTN_ITERATIVE[0]<x<BTN_ITERATIVE[2] and BTN_ITERATIVE[1]<iy<BTN_ITERATIVE[3]:
            switch_dns_mode("iterative")

def switch_dns_mode(mode):
    global dns_mode
    dns_mode = mode

def draw_dns_panel(dt):
    # Select the right data based on mode
    if dns_mode == "recursive":
        dns_nodes = dns_nodes_recursive
        dns_steps = dns_steps_recursive
    else:
        dns_nodes = dns_nodes_iterative
        dns_steps = dns_steps_iterative
        
    cycle = 12.0  # Increased cycle time for more detailed animation
    t = ((dt) % cycle) / cycle * len(dns_steps)
    idx = int(t) % len(dns_steps)
    frac = t - idx
    step = dns_steps[idx]
    
    # Draw title and current step
    glColor3f(1, 1, 0.5)  # Warmer yellow
    draw_text_large(-3.8, 3.0, "DNS Resolution Process")
    
    # Draw caption for current mode
    glColor3f(0.9, 0.9, 1)
    draw_text(-3.5, 2.6, captions[dns_mode])
    
    # Draw current step number and name
    if len(step) > 2:  # If step has description
        glColor3f(0.9, 0.9, 1)  # Light blue for step description
        draw_text_large(-3.8, 2.2, step[2])
    
    # Draw step explanation
    if len(step) > 3:  # If step has explanation caption
        glColor3f(1, 1, 1)  # White for detailed explanation
        draw_text(-3.5, 1.8, step[3])
    
    # Draw nodes with proper positioning
    for name, (x, y) in dns_nodes.items():
        # Determine node color based on its type and activity
        if name in step[:2]:  # Active nodes
            if "Root" in name:
                color = (1.0, 0.7, 0.2)  # Orange for root
            elif "TLD" in name:
                color = (0.2, 0.7, 1.0)  # Blue for TLD
            elif "NS" in name:
                color = (0.8, 0.3, 0.8)  # Purple for NS
            elif "IP" in name:
                color = (1.0, 0.4, 0.4)  # Red for IP
            else:
                color = (1.0, 0.8, 0.3)  # Yellow for active
        else:
            if "Root" in name:
                color = (0.6, 0.4, 0.1)  # Dark orange for inactive root
            elif "TLD" in name:
                color = (0.1, 0.4, 0.6)  # Dark blue for inactive TLD
            elif "NS" in name:
                color = (0.4, 0.2, 0.4)  # Dark purple for inactive NS
            elif "IP" in name:
                color = (0.5, 0.2, 0.2)  # Dark red for inactive IP
            else:
                color = (0.3, 0.5, 0.3)  # Dark green for inactive
        
        # Draw node with improved visual
        node_size = 0.12 if name in step[:2] else 0.09
        draw_node(x, y, node_size, color)
        
        # Draw node label with improved positioning
        glColor3f(1, 1, 1)
        if y < 0:  # Bottom nodes (Your Computer and Local DNS Resolver)
            draw_text(x - len(name)*0.03, y - 0.25, name)
        else:  # Top nodes (servers)
            draw_text(x - len(name)*0.03, y + 0.25, name)
    
    # Draw arrow between active nodes with animation
    if len(step) >= 2:
        p1, p2 = dns_nodes[step[0]], dns_nodes[step[1]]
        
        # Determine arrow color based on step type
        if "Query" in step[2]:  # Request
            arrow_color = (0.2, 0.8, 1.0)  # Blue for requests
        else:  # Response
            arrow_color = (1.0, 0.6, 0.2)  # Orange for responses
            
        # Draw thicker, more visible arrow
        draw_arrow(p1, p2, arrow_color, 3.0)
        
        # Draw moving packet
        px = p1[0] + (p2[0] - p1[0]) * frac
        py = p1[1] + (p2[1] - p1[1]) * frac
        
        # Make packet more visible
        glColor3f(*arrow_color)
        glPushMatrix()
        glTranslatef(px, py, 0)
        glutSolidSphere(0.06, 16, 16)  # Larger, more visible packet
        glPopMatrix()
        
        # Draw a small query text near the moving packet
        if len(step) > 3:
            packet_label = "Query" if "Query" in step[2] else "Response"
            glColor3f(1, 1, 1)
            draw_text(px + 0.1, py + 0.1, packet_label)

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


def reshape(w, h):
    global global_width, global_height
    global_width, global_height = w, h
    glViewport(0, 0, w, h)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    # Increased visible area to make sure all elements are visible
    aspect = w / float(h)
    if aspect >= 1.0:
        gluOrtho2D(-4.0 * aspect, 4.0 * aspect, -4.0, 4.0)
    else:
        gluOrtho2D(-4.0, 4.0, -4.0 / aspect, 4.0 / aspect)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()


def mouse(button,state,x,y):
    if button==GLUT_LEFT_BUTTON and state==GLUT_DOWN:
        manager.mouse(x,y)

# --- Main ---
def main():
    global manager
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB)
    glutInitWindowSize(global_width, global_height)
    glutCreateWindow(b"DNS & HTTP Network Visualization")
    glClearColor(0.12, 0.12, 0.18, 1)  # Nicer dark blue background
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
