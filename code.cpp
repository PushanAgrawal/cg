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
    const Button BTN_MAKE_REQUEST = {362, 350, 662, 420, "Make HTTP Request"};

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

    // HTTP Request Panel Data Structures
    struct HTTPRequest {
        std::string method;
        std::string url;
        std::string jwt;
        std::string jsonBody;
    };

    struct HTTPResponse {
        int statusCode;
        std::string message;
        std::string body;
    };

    // HTTP Request Panel State
    HTTPRequest currentRequest = {
        "GET",
        "/users",
        "valid_jwt_token",
        "{\"name\": \"Alice\"}"
    };

    HTTPResponse currentResponse = {
        200,
        "OK",
        "[{\"id\": 1, \"name\": \"Alice\"}]"
    };

    // UI Elements for HTTP Panel
    struct HTTPPanelButton {
        int x0, y0, x1, y1;
        std::string label;
        bool isSelected;
    };

    // Method selection buttons
    std::vector<HTTPPanelButton> methodButtons = {
        {50, 600, 150, 630, "GET", true},
        {160, 600, 260, 630, "POST", false},
        {270, 600, 370, 630, "PUT", false},
        {380, 600, 480, 630, "DELETE", false}
    };

    // Input field buttons
    HTTPPanelButton urlButton = {50, 550, 450, 580, "URL: /users", false};
    HTTPPanelButton jwtButton = {50, 500, 450, 530, "JWT: valid_jwt_token", false};
    HTTPPanelButton jsonButton = {50, 450, 450, 480, "JSON: {\"name\": \"Alice\"}", false};
    HTTPPanelButton sendButton = {50, 400, 150, 430, "Send Request", false};

    // Response display area
    struct ResponseArea {
        int x0, y0, x1, y1;
        std::string status;
        std::string body;
    } responseArea = {50, 300, 450, 380, "200 OK", "[{\"id\": 1, \"name\": \"Alice\"}]"};

    // Method descriptions
    std::map<std::string, std::string> methodDescriptions = {
        {"GET", "Retrieve data from the server"},
        {"POST", "Create new data on the server"},
        {"PUT", "Update existing data on the server"},
        {"DELETE", "Remove data from the server"}
    };

    // Response code descriptions
    std::map<int, std::string> responseDescriptions = {
        {200, "Success: The request was successful"},
        {201, "Created: The resource was successfully created"},
        {400, "Bad Request: The server could not understand the request"},
        {401, "Unauthorized: Authentication is required"},
        {403, "Forbidden: The server refuses to authorize the request"},
        {404, "Not Found: The requested resource was not found"},
        {500, "Internal Server Error: Something went wrong on the server"}
    };

    // Scene management
    enum Scene { HOME, DNS, HTTP, HTTP_REQUEST };
    Scene currentScene = HOME;

    // Add these global variables after other globals
    bool isEditingField = false;
    std::string* currentEditingField = nullptr;
    std::string currentEditingLabel;

    // Add animation state for request/response
    struct RequestAnimation {
        bool isActive = false;
        float progress = 0.0f;
        std::string from;
        std::string to;
        bool isRequest;
        int statusCode;
        float speed = 0.015f;  // Slower, smoother animation
    };

    RequestAnimation currentAnimation;

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

    // Function to draw a button with selection state
    void drawHTTPButton(const HTTPPanelButton& btn) {
        // Button background
        if (btn.isSelected) {
            glColor3f(0.3f, 0.6f, 0.9f);  // Selected color
        } else {
            glColor3f(0.3f, 0.4f, 0.8f);  // Normal color
        }
        
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
        
        // Button text
        glColor3f(1.0f, 1.0f, 1.0f);
        float textX = btn.x0 + 10;
        float textY = btn.y0 + (btn.y1 - btn.y0) / 2 + 5;
        drawText(textX, textY, btn.label);
    }

    // Function to draw the response area
    void drawResponseArea(const ResponseArea& area) {
        // Background
        glColor3f(0.2f, 0.2f, 0.25f);
        glBegin(GL_QUADS);
        glVertex2f(area.x0, area.y0);
        glVertex2f(area.x1, area.y0);
        glVertex2f(area.x1, area.y1);
        glVertex2f(area.x0, area.y1);
        glEnd();
        
        // Border
        glColor3f(0.4f, 0.5f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(area.x0, area.y0);
        glVertex2f(area.x1, area.y0);
        glVertex2f(area.x1, area.y1);
        glVertex2f(area.x0, area.y1);
        glEnd();
        
        // Status text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(area.x0 + 10, area.y1 - 20, "Response: " + area.status);
        
        // Body text
        glColor3f(0.8f, 0.8f, 1.0f);
        drawText(area.x0 + 10, area.y1 - 40, "Body: " + area.body);
    }

    // Function to start request animation
    void startRequestAnimation(const std::string& from, const std::string& to, bool isRequest, int statusCode) {
        currentAnimation.isActive = true;
        currentAnimation.progress = 0.0f;
        currentAnimation.from = from;
        currentAnimation.to = to;
        currentAnimation.isRequest = isRequest;
        currentAnimation.statusCode = statusCode;
    }

    // Update processHTTPRequest function
    void processHTTPRequest() {
        // Reset response
        currentResponse = {200, "OK", ""};
        
        // Check JWT
        if (currentRequest.jwt != "valid_jwt_token") {
            currentResponse = {401, "Unauthorized", "{\"error\": \"Invalid JWT token\"}"};
            startRequestAnimation("Client", "Server", true, 401);
            return;
        }
        
        // Process based on method and URL
        if (currentRequest.method == "GET") {
            if (currentRequest.url == "/users") {
                currentResponse = {
                    200,
                    "OK",
                    "[{\"id\": 1, \"name\": \"Alice\"}, {\"id\": 2, \"name\": \"Bob\"}]"
                };
                startRequestAnimation("Client", "Server", true, 200);
            } else {
                currentResponse = {
                    404,
                    "Not Found",
                    "{\"error\": \"Resource not found\"}"
                };
                startRequestAnimation("Client", "Server", true, 404);
            }
        }
        else if (currentRequest.method == "POST") {
            if (currentRequest.url == "/users") {
                if (currentRequest.jsonBody.find("{") == std::string::npos) {
                    currentResponse = {
                        400,
                        "Bad Request",
                        "{\"error\": \"Invalid JSON format\"}"
                    };
                    startRequestAnimation("Client", "Server", true, 400);
                } else {
                    currentResponse = {
                        201,
                        "Created",
                        "{\"id\": 3, \"name\": \"New User\"}"
                    };
                    startRequestAnimation("Client", "Server", true, 201);
                }
            }
        }
        else if (currentRequest.method == "PUT") {
            if (currentRequest.url == "/users/1") {
                currentResponse = {
                    200,
                    "OK",
                    "{\"id\": 1, \"name\": \"Updated User\"}"
                };
                startRequestAnimation("Client", "Server", true, 200);
            } else {
                currentResponse = {
                    404,
                    "Not Found",
                    "{\"error\": \"User not found\"}"
                };
                startRequestAnimation("Client", "Server", true, 404);
            }
        }
        else if (currentRequest.method == "DELETE") {
            if (currentRequest.url == "/users/1") {
                currentResponse = {
                    204,
                    "No Content",
                    ""
                };
                startRequestAnimation("Client", "Server", true, 204);
            } else {
                currentResponse = {
                    404,
                    "Not Found",
                    "{\"error\": \"User not found\"}"
                };
                startRequestAnimation("Client", "Server", true, 404);
            }
        }
        
        // Update response area
        responseArea.status = std::to_string(currentResponse.statusCode) + " " + currentResponse.message;
        responseArea.body = currentResponse.body;
    }

    // Function to draw the HTTP panel
    void drawHTTPPanel() {
        // Draw title
        glColor3f(1.0f, 1.0f, 0.5f);
        drawTextLarge(50, 650, "HTTP Request Panel");
        
        // Draw method buttons
        for (auto& btn : methodButtons) {
            drawHTTPButton(btn);
        }
        
        // Draw input fields with editing indicator
        for (auto& btn : {urlButton, jwtButton, jsonButton}) {
            drawHTTPButton(btn);
            if (isEditingField && currentEditingField != nullptr) {
                if ((currentEditingField == &currentRequest.url && btn.label == urlButton.label) ||
                    (currentEditingField == &currentRequest.jwt && btn.label == jwtButton.label) ||
                    (currentEditingField == &currentRequest.jsonBody && btn.label == jsonButton.label)) {
                    // Draw cursor
                    glColor3f(1.0f, 1.0f, 1.0f);
                    float cursorX = btn.x0 + 10 + currentEditingField->length() * 8.0f;
                    float cursorY = btn.y0 + (btn.y1 - btn.y0) / 2;
                    glBegin(GL_LINES);
                    glVertex2f(cursorX, cursorY - 10);
                    glVertex2f(cursorX, cursorY + 10);
                    glEnd();
                }
            }
        }
        
        // Draw send button
        drawHTTPButton(sendButton);
        
        // Draw response area with more details
        drawResponseArea(responseArea);
        
        // Draw method description
        glColor3f(0.9f, 0.9f, 1.0f);
        drawText(50, 350, "Description: " + methodDescriptions[currentRequest.method]);
        
        // Draw response description with more details
        glColor3f(0.9f, 0.9f, 1.0f);
        std::string responseInfo = "Response: " + responseDescriptions[currentResponse.statusCode];
        if (currentResponse.statusCode == 401) {
            responseInfo += "\nTry changing the JWT token to 'valid_jwt_token'";
        } else if (currentResponse.statusCode == 400) {
            responseInfo += "\nMake sure your JSON is properly formatted";
        } else if (currentResponse.statusCode == 404) {
            responseInfo += "\nTry using a different URL or method";
        }
        drawText(50, 330, responseInfo);
        
        // Draw connection to flow visualization
        if (currentResponse.statusCode == 200) {
            glColor3f(0.2f, 0.8f, 0.2f);
            drawText(50, 280, "✓ Request successful - see flow visualization above");
        } else {
            glColor3f(0.8f, 0.2f, 0.2f);
            drawText(50, 280, "✗ Request failed - check the error message above");
        }
    }

    void display() {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        
        double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - startTime;
        
        switch (currentScene) {
            case HOME: {
                // Set up projection for home screen
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw title
                glColor3f(1.0f, 1.0f, 0.5f);
                drawTextLarge(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT - 100, "Network Visualization");
                drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT - 130, "Select a visualization type below");
                
                // Draw buttons with proper coordinates
                drawButton(BTN_DNS);
                drawButton(BTN_HTTP);
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
                // Set up projection for HTTP screen
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw title
                glColor3f(1.0f, 1.0f, 0.5f);
                drawTextLarge(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT - 100, "HTTP Request Flow");
                drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT - 130, "Visualize HTTP request flow or make a request");
                
                // Draw server flow visualization
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - 200);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(-4.0f, 4.0f, -3.0f, 3.0f);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw the server flow visualization
                drawServerFlow(currentTime);
                
                // Reset viewport for buttons
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw buttons
                drawButton(BTN_BACK);
                drawButton(BTN_MAKE_REQUEST);
                break;
            }
                
            case HTTP_REQUEST: {
                // Set up projection for HTTP request screen
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw title
                glColor3f(1.0f, 1.0f, 0.5f);
                drawTextLarge(50, WINDOW_HEIGHT - 50, "HTTP Request Panel");
                
                // Draw HTTP panel
                drawHTTPPanel();
                
                // Draw animation area
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - 300);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(-4.0f, 4.0f, -3.0f, 3.0f);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw request animation if active
                if (currentAnimation.isActive) {
                    drawRequestAnimation();
                }
                
                // Reset viewport for buttons
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                // Draw Back button
                drawButton(BTN_BACK);
                break;
            }
        }
        
        glutSwapBuffers();  // Ensure double buffering is used
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

    // Add keyboard input handling
    void keyboard(unsigned char key, int x, int y) {
        if (isEditingField && currentEditingField != nullptr) {
            if (key == 13) { // Enter key
                isEditingField = false;
                currentEditingField = nullptr;
            } else if (key == 8) { // Backspace
                if (!currentEditingField->empty()) {
                    currentEditingField->pop_back();
                }
            } else if (key >= 32 && key <= 126) { // Printable characters
                currentEditingField->push_back(key);
            }
            
            // Update button labels
            if (currentEditingField == &currentRequest.url) {
                urlButton.label = "URL: " + currentRequest.url;
            } else if (currentEditingField == &currentRequest.jwt) {
                jwtButton.label = "JWT: " + currentRequest.jwt;
            } else if (currentEditingField == &currentRequest.jsonBody) {
                jsonButton.label = "JSON: " + currentRequest.jsonBody;
            }
            
            glutPostRedisplay();
        }
    }

    // Add this function to handle field editing
    void startEditingField(std::string* field, const std::string& label) {
        isEditingField = true;
        currentEditingField = field;
        currentEditingLabel = label;
    }

    // Update the mouse function to handle field editing
    void mouse(int button, int state, int x, int y) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            y = WINDOW_HEIGHT - y;  // Convert to OpenGL coordinates
            
            switch (currentScene) {
                case HOME:
                    // Check DNS button
                    if (x >= BTN_DNS.x0 && x <= BTN_DNS.x1 && 
                        y >= BTN_DNS.y0 && y <= BTN_DNS.y1) {
                        currentScene = DNS;
                    }
                    // Check HTTP button
                    else if (x >= BTN_HTTP.x0 && x <= BTN_HTTP.x1 && 
                             y >= BTN_HTTP.y0 && y <= BTN_HTTP.y1) {
                        currentScene = HTTP;
                    }
                    break;
                    
                case DNS:
                    if (x >= BTN_BACK.x0 && x <= BTN_BACK.x1 && 
                        y >= BTN_BACK.y0 && y <= BTN_BACK.y1) {
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
                    if (x >= BTN_BACK.x0 && x <= BTN_BACK.x1 && 
                        y >= BTN_BACK.y0 && y <= BTN_BACK.y1) {
                        currentScene = HOME;
                    }
                    else if (x >= BTN_MAKE_REQUEST.x0 && x <= BTN_MAKE_REQUEST.x1 && 
                             y >= BTN_MAKE_REQUEST.y0 && y <= BTN_MAKE_REQUEST.y1) {
                        currentScene = HTTP_REQUEST;
                    }
                    break;
                
                case HTTP_REQUEST:
                    if (x >= BTN_BACK.x0 && x <= BTN_BACK.x1 && 
                        y >= BTN_BACK.y0 && y <= BTN_BACK.y1) {
                        currentScene = HTTP;
                    }
                    
                    // Handle HTTP panel buttons
                    for (auto& btn : methodButtons) {
                        if (x >= btn.x0 && x <= btn.x1 && 
                            y >= btn.y0 && y <= btn.y1) {
                            for (auto& b : methodButtons) {
                                b.isSelected = false;
                            }
                            btn.isSelected = true;
                            currentRequest.method = btn.label;
                            break;
                        }
                    }
                    
                    if (x >= urlButton.x0 && x <= urlButton.x1 && 
                        y >= urlButton.y0 && y <= urlButton.y1) {
                        startEditingField(&currentRequest.url, "URL");
                    }
                    else if (x >= jwtButton.x0 && x <= jwtButton.x1 && 
                             y >= jwtButton.y0 && y <= jwtButton.y1) {
                        startEditingField(&currentRequest.jwt, "JWT");
                    }
                    else if (x >= jsonButton.x0 && x <= jsonButton.x1 && 
                             y >= jsonButton.y0 && y <= jsonButton.y1) {
                        startEditingField(&currentRequest.jsonBody, "JSON");
                    }
                    else if (x >= sendButton.x0 && x <= sendButton.x1 && 
                             y >= sendButton.y0 && y <= sendButton.y1) {
                        processHTTPRequest();
                    }
                    break;
            }
            
            glutPostRedisplay();  // Ensure screen updates after button clicks
        }
    }

    void timer(int value) {
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);  // 60 FPS
    }

    // Add function to draw server flow
    void drawServerFlow(double currentTime) {
        // Draw title
        glColor3f(1.0f, 1.0f, 0.5f);
        drawTextLarge(-3.8f, 2.5f, "Server Request Flow");
        
        // Draw nodes
        std::map<std::string, std::pair<float, float>> http_nodes = {
            {"Client", {-2.0f, 0.0f}},
            {"LB", {-1.0f, 0.0f}},
            {"Server", {0.0f, 0.0f}},
            {"App", {1.0f, 0.0f}},
            {"DB", {2.0f, 0.0f}}
        };
        
        // Draw nodes and connections
        for (const auto& [name, pos] : http_nodes) {
            drawNode(pos.first, pos.second, 0.2f, 0.3f, 0.4f, 0.8f);
            drawText(pos.first - 0.2f, pos.second - 0.3f, name);
        }
        
        // Draw arrows between nodes
        for (size_t i = 0; i < http_nodes.size() - 1; i++) {
            auto it1 = std::next(http_nodes.begin(), i);
            auto it2 = std::next(http_nodes.begin(), i + 1);
            drawArrow(it1->second.first, it1->second.second,
                     it2->second.first, it2->second.second,
                     0.2f, 0.8f, 1.0f);
        }
    }

    // Add function to draw request animation
    void drawRequestAnimation() {
        // Update animation progress
        currentAnimation.progress += currentAnimation.speed;
        if (currentAnimation.progress >= 1.0f) {
            currentAnimation.isActive = false;
        }
        
        // Draw animation
        float x1 = -2.0f;
        float y1 = 0.0f;
        float x2 = 2.0f;
        float y2 = 0.0f;
        
        // Calculate color based on status code
        float ar, ag, ab;
        if (currentAnimation.statusCode >= 200 && currentAnimation.statusCode < 300) {
            ar = 0.2f; ag = 0.8f; ab = 0.2f;  // Green for success
        } else if (currentAnimation.statusCode >= 400 && currentAnimation.statusCode < 500) {
            ar = 0.8f; ag = 0.2f; ab = 0.2f;  // Red for client error
        } else if (currentAnimation.statusCode >= 500) {
            ar = 1.0f; ag = 0.6f; ab = 0.2f;  // Orange for server error
        } else {
            ar = 0.2f; ag = 0.8f; ab = 1.0f;  // Blue for other
        }
        
        // Draw arrow with glow
        glColor4f(ar, ag, ab, 0.2f);
        glLineWidth(8.0f);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
        
        // Draw main arrow
        glColor3f(ar, ag, ab);
        glLineWidth(4.0f);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
        
        // Draw moving packet with trail
        float px = x1 + (x2 - x1) * currentAnimation.progress;
        float py = y1 + (y2 - y1) * currentAnimation.progress;
        
        // Draw packet trail
        for (int i = 0; i < 5; i++) {
            float trailProgress = currentAnimation.progress - (i * 0.05f);
            if (trailProgress > 0) {
                float trailX = x1 + (x2 - x1) * trailProgress;
                float trailY = y1 + (y2 - y1) * trailProgress;
                glColor4f(ar, ag, ab, 0.1f - (i * 0.02f));
                glPushMatrix();
                glTranslatef(trailX, trailY, 0);
                glutSolidSphere(0.1f - (i * 0.01f), 16, 16);
                glPopMatrix();
            }
        }
        
        // Draw main packet
        glColor3f(ar, ag, ab);
        glPushMatrix();
        glTranslatef(px, py, 0);
        glutSolidSphere(0.08f, 32, 32);
        glPopMatrix();
        
        // Draw status code
        glColor3f(0.2f, 0.2f, 0.25f);
        std::string statusText = std::to_string(currentAnimation.statusCode);
        float textWidth = statusText.length() * 0.1f;
        glBegin(GL_QUADS);
        glVertex2f(px - textWidth/2 - 0.05f, py + 0.15f);
        glVertex2f(px + textWidth/2 + 0.05f, py + 0.15f);
        glVertex2f(px + textWidth/2 + 0.05f, py + 0.25f);
        glVertex2f(px - textWidth/2 - 0.05f, py + 0.25f);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(px - textWidth/2, py + 0.2f, statusText);
    }

    int main(int argc, char** argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  // Enable double buffering and depth
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutCreateWindow("DNS & HTTP Network Visualization");
        
        glClearColor(0.12f, 0.12f, 0.18f, 1.0f);
        
        // Enable smooth shading
        glShadeModel(GL_SMOOTH);
        
        // Enable blending for smooth transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutMouseFunc(mouse);
        glutKeyboardFunc(keyboard);
        glutTimerFunc(16, timer, 0);  // 60 FPS
        
        startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        
        glutMainLoop();
        return 0;
    } 