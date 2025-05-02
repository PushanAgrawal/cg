#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <cmath>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <initializer_list> 
// Window dimensions
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Global variables
double startTime = 0;
std::string dnsMode = "recursive";
float h_spacing = 0.4f;  // Horizontal spacing
float v_base = 0.5f;     // Vertical base
float v_spacing = 0.6f;  // Vertical spacing

// Button definitions (screen coords)
struct Button {
    int x0, y0, x1, y1;
    std::string label;
};

// Adjust button dimensions to better fit text
const Button BTN_DNS = {362, 350, 662, 420, "DNS Resolution Process"};
const Button BTN_HTTP = {362, 250, 662, 320, "HTTP Request Flow"};
const Button BTN_BACK = {20, 20, 120, 60, "Back"};
const Button BTN_RECURSIVE = {200, 700, 400, 740, "Recursive Mode"};
const Button BTN_ITERATIVE = {600, 700, 800, 740, "Iterative Mode"};

// DNS Data structures
struct DNSNode {
    float x, y;
};

struct DNSStep {
    std::string from;
    std::string to;
    std::string title;
    std::string description;
};

// DNS Data - Recursive Mode
std::map<std::string, DNSNode> dns_nodes_recursive = {
    {"Your Computer", {0, -2.0f}},
    {"Local DNS Resolver", {0, -0.5f}},
    {"Root Server", {-3.0f, 1.0f}},
    {".com TLD Server", {-1.5f, 1.0f}},
    {"google.com NS", {0, 1.0f}},
    {"www.google.com", {1.5f, 1.0f}},
    {"IP: 142.250.190.36", {3.0f, 1.0f}}
};

std::vector<DNSStep> dns_steps_recursive = {
    {"Your Computer", "Local DNS Resolver", 
     "Step 1: Request for www.google.com", 
     "User sends lookup request to local resolver"},
    {"Local DNS Resolver", "Root Server",
     "Step 2: Root Server Query",
     "Resolver asks root server: Where is .com?"},
    {"Root Server", "Local DNS Resolver",
     "Step 3: TLD Server Referral",
     "Root returns referral to .com TLD servers"},
    {"Local DNS Resolver", ".com TLD Server",
     "Step 4: TLD Server Query",
     "Resolver asks TLD: Where is google.com?"},
    {".com TLD Server", "Local DNS Resolver",
     "Step 5: Authoritative NS Referral",
     "TLD returns referral to google.com name servers"},
    {"Local DNS Resolver", "google.com NS",
     "Step 6: Authoritative Server Query",
     "Resolver asks: What's the IP for www.google.com?"},
    {"google.com NS", "Local DNS Resolver",
     "Step 7: IP Address Response",
     "Name server returns IP address 142.250.190.36"},
    {"Local DNS Resolver", "Your Computer",
     "Step 8: Final Response",
     "Resolver returns the IP to your computer"}
};

// DNS Data - Iterative Mode
std::map<std::string, DNSNode> dns_nodes_iterative = {
    {"Your Computer", {0, -2.0f}},
    {"Root Server", {-3.0f, 1.0f}},
    {".com TLD Server", {-1.5f, 1.0f}},
    {"google.com NS", {0, 1.0f}},
    {"www.google.com", {1.5f, 1.0f}},
    {"IP: 142.250.190.36", {3.0f, 1.0f}}
};

std::vector<DNSStep> dns_steps_iterative = {
    {"Your Computer", "Root Server",
     "Step 1: Root Server Query",
     "User asks root server: Where is www.google.com?"},
    {"Root Server", "Your Computer",
     "Step 2: TLD Server Referral",
     "Root server: I don't know, ask .com TLD servers"},
    {"Your Computer", ".com TLD Server",
     "Step 3: TLD Server Query",
     "User asks .com TLD: Where is www.google.com?"},
    {".com TLD Server", "Your Computer",
     "Step 4: Authoritative NS Referral",
     "TLD: I don't know, ask google.com name servers"},
    {"Your Computer", "google.com NS",
     "Step 5: Authoritative Server Query",
     "User asks: What's the IP for www.google.com?"},
    {"google.com NS", "Your Computer",
     "Step 6: IP Address Response",
     "Name server returns IP address 142.250.190.36"}
};

// Add HTTP data structures after DNS data structures
struct HTTPStep {
    std::string from;
    std::string to;
    std::string title;
    std::string description;
    bool isRequest;
};

// HTTP Data
std::vector<HTTPStep> http_steps = {
    {"Client", "LB", 
     "Step 1: Initial HTTP Request", 
     "Client sends HTTP GET request to Load Balancer", true},
    {"LB", "Server", 
     "Step 2: Load Balancer Forwarding", 
     "Load Balancer routes request to least busy server", true},
    {"Server", "App", 
     "Step 3: Application Processing", 
     "Web server forwards request to application layer", true},
    {"App", "DB", 
     "Step 4: Database Query", 
     "Application queries database for requested data", true},
    {"DB", "App", 
     "Step 5: Database Response", 
     "Database returns requested data to application", false},
    {"App", "Server", 
     "Step 6: Response Generation", 
     "Application generates HTTP response", false},
    {"Server", "LB", 
     "Step 7: Server Response", 
     "Server sends response back through Load Balancer", false},
    {"LB", "Client", 
     "Step 8: Final Response", 
     "Client receives HTTP response with requested data", false}
};

// Scene management
enum Scene { HOME, DNS, HTTP };
Scene currentScene = HOME;

// Helper functions
void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_10) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void drawTextLarge(float x, float y, const std::string& text) {
    drawText(x, y, text, GLUT_BITMAP_HELVETICA_18);
}

void drawButton(const Button& btn) {
    glColor3f(0.3f, 0.4f, 0.8f);  // Nice blue color
    glBegin(GL_QUADS);
    glVertex2f(btn.x0, btn.y0);
    glVertex2f(btn.x1, btn.y0);
    glVertex2f(btn.x1, btn.y1);
    glVertex2f(btn.x0, btn.y1);
    glEnd();
    
    // Button border
    glColor3f(0.4f, 0.5f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn.x0, btn.y0);
    glVertex2f(btn.x1, btn.y0);
    glVertex2f(btn.x1, btn.y1);
    glVertex2f(btn.x0, btn.y1);
    glEnd();
    
    // Center text in button
    glColor3f(1.0f, 1.0f, 1.0f);
    void* font = GLUT_BITMAP_HELVETICA_18;
    int text_width = 0;
    for (char c : btn.label) {
        text_width += glutBitmapWidth(font, c);
    }
    float text_x = btn.x0 + (btn.x1 - btn.x0 - text_width) / 2.0f;
    float text_y = btn.y0 + (btn.y1 - btn.y0 - 10) / 2.0f + 10;  // 10 is approximate text height
    
    glRasterPos2f(text_x, text_y);
    for (char c : btn.label) {
        glutBitmapCharacter(font, c);
    }
}

void drawNode(float x, float y, float radius, float r, float g, float b) {
    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glutSolidSphere(radius, 24, 24);
    
    // Highlight effect
    glColor3f(r + 0.2f, g + 0.2f, b + 0.2f);
    glTranslatef(radius * 0.3f, radius * 0.3f, 0);
    glutSolidSphere(radius * 0.3f, 16, 16);
    glPopMatrix();
}

void drawArrow(float x1, float y1, float x2, float y2, float r, float g, float b, float width = 1.0f) {
    glColor3f(r, g, b);
    glLineWidth(width);
    
    // Draw main line
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    
    // Draw arrowhead
    float dx = x2 - x1;
    float dy = y2 - y1;
    float angle = atan2(dy, dx);
    float size = 0.15f;
    
    for (float sign : {1.0f, -1.0f}) {
        float a = angle + sign * 0.4f;
        float x = x2 - size * cos(a);
        float y = y2 - size * sin(a);
        glBegin(GL_LINES);
        glVertex2f(x2, y2);
        glVertex2f(x, y);
        glEnd();
    }
}

void drawDNSPanel(double dt) {
    auto& dns_nodes = (dnsMode == "recursive") ? dns_nodes_recursive : dns_nodes_iterative;
    auto& dns_steps = (dnsMode == "recursive") ? dns_steps_recursive : dns_steps_iterative;
    
    double cycle = 12.0;
    double t = fmod(dt, cycle) / cycle * dns_steps.size();
    int idx = static_cast<int>(t) % dns_steps.size();
    float frac = t - idx;
    
    // Draw title
    glColor3f(1.0f, 1.0f, 0.5f);
    drawTextLarge(-3.8f, 3.0f, "DNS Resolution Process");
    
    // Draw mode caption
    glColor3f(0.9f, 0.9f, 1.0f);
    std::string caption = (dnsMode == "recursive") ?
        "Recursive DNS: Local resolver does all the work on behalf of client" :
        "Iterative DNS: Client does all the work, contacting each server in sequence";
    drawText(-3.5f, 2.6f, caption);
    
    // Draw current step
    const auto& step = dns_steps[idx];
    glColor3f(0.9f, 0.9f, 1.0f);
    drawTextLarge(-3.8f, 2.2f, step.title);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-3.5f, 1.8f, step.description);
    
    // Draw nodes
    for (const auto& [name, node] : dns_nodes) {
        bool isActive = (name == step.from || name == step.to);
        float r, g, b;
        
        if (isActive) {
            if (name.find("Root") != std::string::npos) {
                r = 1.0f; g = 0.7f; b = 0.2f;  // Orange for root
            } else if (name.find("TLD") != std::string::npos) {
                r = 0.2f; g = 0.7f; b = 1.0f;  // Blue for TLD
            } else if (name.find("NS") != std::string::npos) {
                r = 0.8f; g = 0.3f; b = 0.8f;  // Purple for NS
            } else if (name.find("IP") != std::string::npos) {
                r = 1.0f; g = 0.4f; b = 0.4f;  // Red for IP
            } else {
                r = 1.0f; g = 0.8f; b = 0.3f;  // Yellow for active
            }
        } else {
            if (name.find("Root") != std::string::npos) {
                r = 0.6f; g = 0.4f; b = 0.1f;
            } else if (name.find("TLD") != std::string::npos) {
                r = 0.1f; g = 0.4f; b = 0.6f;
            } else if (name.find("NS") != std::string::npos) {
                r = 0.4f; g = 0.2f; b = 0.4f;
            } else if (name.find("IP") != std::string::npos) {
                r = 0.5f; g = 0.2f; b = 0.2f;
            } else {
                r = 0.3f; g = 0.5f; b = 0.3f;
            }
        }
        
        float nodeSize = isActive ? 0.12f : 0.09f;
        drawNode(node.x, node.y, nodeSize, r, g, b);
        
        // Draw node label
        glColor3f(1.0f, 1.0f, 1.0f);
        float textY = (node.y < 0) ? node.y - 0.25f : node.y + 0.25f;
        drawText(node.x - name.length() * 0.04f, textY, name);
    }
    
    // Draw arrow and packet
    const auto& fromNode = dns_nodes[step.from];
    const auto& toNode = dns_nodes[step.to];
    
    bool isQuery = step.title.find("Query") != std::string::npos;
    float ar = isQuery ? 0.2f : 1.0f;
    float ag = isQuery ? 0.8f : 0.6f;
    float ab = isQuery ? 1.0f : 0.2f;
    
    drawArrow(fromNode.x, fromNode.y, toNode.x, toNode.y, ar, ag, ab, 3.0f);
    
    // Draw moving packet
    float px = fromNode.x + (toNode.x - fromNode.x) * frac;
    float py = fromNode.y + (toNode.y - fromNode.y) * frac;
    
    glColor3f(ar, ag, ab);
    glPushMatrix();
    glTranslatef(px, py, 0);
    glutSolidSphere(0.06f, 16, 16);
    glPopMatrix();
    
    // Draw packet label
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(px + 0.15f, py + 0.15f, isQuery ? "Query" : "Response");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - startTime;
    
    switch (currentScene) {
        case HOME: {
            glColor3f(1.0f, 1.0f, 0.5f);
            drawTextLarge(-2.0f, 1.5f, "Network Visualization");
            drawText(-2.0f, 1.2f, "Select a visualization type below");
            
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            drawButton(BTN_DNS);
            drawButton(BTN_HTTP);
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            break;
        }
            
        case DNS: {
            drawDNSPanel(currentTime);
            
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            drawButton(BTN_BACK);
            drawButton(BTN_RECURSIVE);
            drawButton(BTN_ITERATIVE);
            
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(450, 675);
            std::string modeText = "Current Mode: " + dnsMode;
            for (char c : modeText) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
            }
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            break;
        }
            
        case HTTP: {
            // Set up the viewport and projection first
            glLoadIdentity();
            
            // Draw HTTP visualization
            glColor3f(1.0f, 1.0f, 0.5f);
            drawTextLarge(-2.0f, 1.5f, "HTTP Request Flow");
            
            // Draw description of current step
            double cycle = 12.0;  // 12 second cycle
            double t = fmod(currentTime, cycle) / cycle * http_steps.size();
            int idx = static_cast<int>(t) % http_steps.size();
            float frac = t - idx;
            
            const auto& step = http_steps[idx];
            
            // Draw step information
            glColor3f(0.9f, 0.9f, 1.0f);
            drawTextLarge(-2.0f, 1.2f, step.title);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(-1.8f, 0.9f, step.description);
            
            // Draw nodes with adjusted positions
            std::map<std::string, std::pair<float, float>> http_nodes = {
                {"Client", {-2.0f, 0.0f}},
                {"LB", {-1.0f, 0.0f}},
                {"Server", {0.0f, 0.0f}},
                {"App", {1.0f, 0.0f}},
                {"DB", {2.0f, 0.0f}}
            };
            
            // Draw nodes with active highlighting
            for (const auto& [name, pos] : http_nodes) {
                bool isActive = (name == step.from || name == step.to);
                float r = isActive ? 0.3f : 0.2f;
                float g = isActive ? 0.6f : 0.5f;
                float b = isActive ? 0.9f : 0.8f;
                
                glColor3f(r, g, b);
                glPushMatrix();
                glTranslatef(pos.first, pos.second, 0);
                glutSolidCube(0.3f);
                glPopMatrix();
                
                glColor3f(1.0f, 1.0f, 1.0f);
                drawText(pos.first - 0.2f, pos.second - 0.4f, name);
            }
            
            // Draw animated arrow between active nodes
            if (http_nodes.count(step.from) && http_nodes.count(step.to)) {
                const auto& fromPos = http_nodes[step.from];
                const auto& toPos = http_nodes[step.to];
                
                // Different colors for request vs response
                float ar = step.isRequest ? 0.2f : 1.0f;
                float ag = step.isRequest ? 0.8f : 0.6f;
                float ab = step.isRequest ? 1.0f : 0.2f;
                
                drawArrow(fromPos.first, fromPos.second, 
                         toPos.first, toPos.second, 
                         ar, ag, ab, 3.0f);
                
                // Draw moving packet
                float px = fromPos.first + (toPos.first - fromPos.first) * frac;
                float py = fromPos.second + (toPos.second - fromPos.second) * frac;
                
                glColor3f(ar, ag, ab);
                glPushMatrix();
                glTranslatef(px, py, 0);
                glutSolidSphere(0.06f, 16, 16);
                glPopMatrix();
                
                // Draw packet label
                glColor3f(1.0f, 1.0f, 1.0f);
                drawText(px + 0.15f, py + 0.15f, 
                        step.isRequest ? "Request" : "Response");
            }
            
            // Draw Back button in screen coordinates
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            drawButton(BTN_BACK);
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            break;
        }
    }
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = w / static_cast<float>(h);
    if (aspect >= 1.0f) {
        gluOrtho2D(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f);
    } else {
        gluOrtho2D(-4.0f, 4.0f, -4.0f / aspect, 4.0f / aspect);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        y = WINDOW_HEIGHT - y;  // Convert to OpenGL coordinates
        
        switch (currentScene) {
            case HOME:
                if (x >= BTN_DNS.x0 && x <= BTN_DNS.x1 && y >= BTN_DNS.y0 && y <= BTN_DNS.y1) {
                    currentScene = DNS;
                } else if (x >= BTN_HTTP.x0 && x <= BTN_HTTP.x1 && y >= BTN_HTTP.y0 && y <= BTN_HTTP.y1) {
                    currentScene = HTTP;
                }
                break;
                
            case DNS:
                if (x >= BTN_BACK.x0 && x <= BTN_BACK.x1 && y >= BTN_BACK.y0 && y <= BTN_BACK.y1) {
                    currentScene = HOME;
                } else if (x >= BTN_RECURSIVE.x0 && x <= BTN_RECURSIVE.x1 && 
                          y >= BTN_RECURSIVE.y0 && y <= BTN_RECURSIVE.y1) {
                    dnsMode = "recursive";
                } else if (x >= BTN_ITERATIVE.x0 && x <= BTN_ITERATIVE.x1 && 
                          y >= BTN_ITERATIVE.y0 && y <= BTN_ITERATIVE.y1) {
                    dnsMode = "iterative";
                }
                break;
                
            case HTTP:
                if (x >= BTN_BACK.x0 && x <= BTN_BACK.x1 && y >= BTN_BACK.y0 && y <= BTN_BACK.y1) {
                    currentScene = HOME;
                }
                break;
        }
    }
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // 60 FPS
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("DNS & HTTP Network Visualization");
    
    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);  // Nice dark blue background
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutTimerFunc(0, timer, 0);
    
    startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    
    glutMainLoop();
    return 0;
} 