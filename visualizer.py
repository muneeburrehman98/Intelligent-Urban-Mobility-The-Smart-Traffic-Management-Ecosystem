import pygame
import sys
import math
import random

# --- CONSTANTS ---
WIDTH, HEIGHT = 850, 850
BG_COLOR = (30, 30, 30) # Dark Background
ROAD_COLOR = (60, 60, 60) # Dark Grey Asphalt
LANE_COLOR = (255, 255, 255) # White Dashed
NODE_COLOR = (60, 60, 60) # Same as road
VEHICLE_MOVING_COLOR = (0, 255, 0) # Green
VEHICLE_STOPPED_COLOR = (255, 0, 0) # Red
VEHICLE_EMERGENCY_COLOR = (0, 100, 255) # Blue
LIGHT_GREEN = (0, 255, 0)
LIGHT_RED = (255, 0, 0)

ROAD_WIDTH = 30
CAR_LENGTH = 12
CAR_WIDTH = 6

def parse_file(filename):
    nodes = {}
    edges = []
    frames = []
    
    current_frame = None
    parsing_nodes = False
    parsing_edges = False

    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: Could not find {filename}. Did you run 'simulation.exe > output.txt'?")
        sys.exit()

    for line in lines:
        line = line.strip()
        parts = line.split()
        if not parts: continue

        if line == "NODES":
            parsing_nodes = True
            parsing_edges = False
            continue
        elif line == "EDGES":
            parsing_nodes = False
            parsing_edges = True
            continue
        elif line == "END_GRAPH":
            parsing_nodes = False
            parsing_edges = False
            continue

        if parsing_nodes and len(parts) == 3:
            nodes[int(parts[0])] = (float(parts[1]), float(parts[2]))
        elif parsing_edges and len(parts) == 3:
            edges.append((int(parts[0]), int(parts[1]), int(parts[2])))

        if line.startswith("STATE"):
            current_frame = {'vehicles': [], 'lights': {}}
        elif line == "END_STATE":
            if current_frame:
                frames.append(current_frame)
            current_frame = None
        elif current_frame is not None:
            if parts[0] == 'V':
                # V ID RoadID Position
                vid = int(parts[1])
                road_id = int(parts[2])
                pos = float(parts[3])
                current_frame['vehicles'].append((road_id, pos, vid))
            elif parts[0] == 'L':
                # L NodeID GreenRoadID
                nid = int(parts[1])
                road_id = int(parts[2])
                current_frame['lights'][nid] = road_id

    return nodes, edges, frames

def draw_dashed_line(surf, color, start_pos, end_pos, width=1, dash_length=10):
    x1, y1 = start_pos
    x2, y2 = end_pos
    dl = dash_length

    if (x1 == x2):
        ycoords = [y for y in range(int(y1), int(y2), dl if y1 < y2 else -dl)]
        xcoords = [x1] * len(ycoords)
    elif (y1 == y2):
        xcoords = [x for x in range(int(x1), int(x2), dl if x1 < x2 else -dl)]
        ycoords = [y1] * len(xcoords)
    else:
        a = abs(x2 - x1)
        b = abs(y2 - y1)
        c = round(math.sqrt(a**2 + b**2))
        dx = dl * a / c
        dy = dl * b / c
        
        xcoords = [x for x in range(int(x1), int(x2), int(dx) if x1 < x2 else -int(dx))]
        ycoords = [y for y in range(int(y1), int(y2), int(dy) if y1 < y2 else -int(dy))]

    next_coords = list(zip(xcoords[1::2], ycoords[1::2]))
    last_coords = list(zip(xcoords[0::2], ycoords[0::2]))
    for (x1, y1), (x2, y2) in zip(last_coords, next_coords):
        start = (round(x1), round(y1))
        end = (round(x2), round(y2))
        pygame.draw.line(surf, color, start, end, width)

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Traffic Simulation: Arcade Physics")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont('Arial', 12, bold=True)

    print("Loading output.txt...")
    nodes, edges, frames = parse_file("output.txt")
    print(f"Loaded {len(nodes)} nodes, {len(edges)} roads.")

    # Pre-calculate road geometry
    road_map = {} 
    
    for r in edges:
        rid, u, v = r
        if u not in nodes or v not in nodes: continue
        
        p1 = nodes[u]
        p2 = nodes[v]
        
        dx = p2[0] - p1[0]
        dy = p2[1] - p1[1]
        length = math.sqrt(dx*dx + dy*dy)
        angle = math.atan2(dy, dx)
        
        # Offset to the RIGHT for bidirectional traffic
        offset = 10.0 
        
        if length > 0:
            nx = -dy / length
            ny = dx / length
            
            sx = p1[0] + nx * offset
            sy = p1[1] + ny * offset
            ex = p2[0] + nx * offset
            ey = p2[1] + ny * offset
            
            road_map[rid] = ((sx, sy), (ex, ey), angle, length, v) # Store DestNode V

    frame_idx = 0
    last_update = pygame.time.get_ticks()
    FRAME_DELAY = 100 

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False

        now = pygame.time.get_ticks()
        if now - last_update > FRAME_DELAY:
            frame_idx = (frame_idx + 1) % len(frames) if frames else 0
            last_update = now

        screen.fill(BG_COLOR)
        
        current_data = frames[frame_idx] if frames else {'lights': {}, 'vehicles': []}

        # 1. Draw Roads
        for rid, geom in road_map.items():
            start, end, angle, length, dest_node = geom
            pygame.draw.line(screen, ROAD_COLOR, start, end, ROAD_WIDTH)
            draw_dashed_line(screen, LANE_COLOR, start, end, width=1)

        # 2. Draw Traffic Lights (Stop Bars)
        for rid, geom in road_map.items():
            start, end, angle, length, dest_node = geom
            
            light_status = current_data['lights'].get(dest_node, -1)
            
            # Default Red
            color = LIGHT_RED
            if light_status == rid:
                color = LIGHT_GREEN
            
            # Draw bar at the end of the road
            # Perpendicular vector
            nx = -math.sin(angle)
            ny = math.cos(angle)
            
            # Position: slightly back from end
            stop_dist = 15
            cx = end[0] - math.cos(angle) * stop_dist
            cy = end[1] - math.sin(angle) * stop_dist
            
            w = ROAD_WIDTH / 2
            p1 = (cx + nx * w, cy + ny * w)
            p2 = (cx - nx * w, cy - ny * w)
            
            pygame.draw.line(screen, color, p1, p2, 4)

        # 3. Draw Vehicles
        for (road_id, pos_on_road, vid) in current_data['vehicles']:
            if road_id in road_map:
                start, end, angle, length, dest_node = road_map[road_id]
                
                if length > 0:
                    ratio = pos_on_road / length
                    if ratio > 1.0: ratio = 1.0
                    
                    vx = start[0] + (end[0] - start[0]) * ratio
                    vy = start[1] + (end[1] - start[1]) * ratio
                    
                    # Color Logic
                    random.seed(vid) 
                    r = random.randint(100, 255)
                    g = random.randint(100, 255)
                    b = random.randint(100, 255)
                    c = (r, g, b)
                    
                    if vid % 50 == 0: 
                        c = VEHICLE_EMERGENCY_COLOR
                    elif ratio > 0.9: 
                         light_status = current_data['lights'].get(dest_node, -1)
                         if light_status != road_id:
                             c = VEHICLE_STOPPED_COLOR 
                    
                    # Draw Car Rectangle
                    car_surf = pygame.Surface((CAR_LENGTH, CAR_WIDTH), pygame.SRCALPHA)
                    pygame.draw.rect(car_surf, c, (0, 0, CAR_LENGTH, CAR_WIDTH))
                    
                    angle_deg = -math.degrees(angle)
                    rotated_car = pygame.transform.rotate(car_surf, angle_deg)
                    rect = rotated_car.get_rect(center=(vx, vy))
                    screen.blit(rotated_car, rect)

        # Info
        info_text = f"Frame: {frame_idx}/{len(frames)} | Vehicles: {len(current_data['vehicles'])}"
        screen.blit(font.render(info_text, True, (255, 255, 255)), (10, 10))

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()