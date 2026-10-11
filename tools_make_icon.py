import math
from PIL import Image, ImageDraw, ImageFilter

def create_ghost_icon(size):
    # Create RGBA canvas
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    scale = size / 256.0
    
    def s(val):
        return val * scale

    # Center and radii
    cx, cy = size / 2.0, size / 2.0
    
    # 1. Outer Dark Hexagonal/Octagonal Shield Badge
    # Draw smooth rounded polygon badge
    pad = s(12)
    badge_box = [pad, pad, size - pad, size - pad]
    badge_bg = (15, 18, 24, 255)       # Obsidian dark
    border_color = (42, 50, 68, 255)   # Metallic border
    cyan_glow = (0, 220, 255, 255)     # Glowing cyan
    
    corner_r = s(28)
    draw.rounded_rectangle(badge_box, radius=corner_r, fill=badge_bg, outline=border_color, width=max(1, int(s(4))))
    
    # Inner metallic edge
    inner_pad = pad + s(6)
    inner_box = [inner_pad, inner_pad, size - inner_pad, size - inner_pad]
    draw.rounded_rectangle(inner_box, radius=corner_r - s(4), outline=(28, 36, 48, 200), width=max(1, int(s(2))))
    
    # 2. Ghost Silhouette / Mask (Sleek Gamer Visor Style)
    # Head curve
    head_top = cy - s(62)
    head_w = s(52)
    head_h = s(55)
    
    # Draw ghost dome
    ghost_poly = [
        (cx - head_w, cy - s(10)),
        (cx - head_w, cy - s(40)),
        (cx - s(35), head_top),
        (cx, head_top - s(8)),
        (cx + s(35), head_top),
        (cx + head_w, cy - s(40)),
        (cx + head_w, cy - s(10)),
        # Bottom flowing tentacles / folds
        (cx + head_w - s(12), cy + s(45)),
        (cx + s(25), cy + s(32)),
        (cx + s(10), cy + s(52)),
        (cx, cy + s(36)),
        (cx - s(10), cy + s(52)),
        (cx - s(25), cy + s(32)),
        (cx - head_w + s(12), cy + s(45)),
    ]
    
    ghost_fill = (225, 235, 245, 240)
    draw.polygon(ghost_poly, fill=ghost_fill)
    
    # 3. Glowing Cyber Visor / Eyes (Sharp angled gamer eyes)
    eye_y = cy - s(18)
    # Left eye
    left_eye = [
        (cx - s(36), eye_y - s(6)),
        (cx - s(12), eye_y),
        (cx - s(20), eye_y + s(10)),
        (cx - s(38), eye_y + s(4))
    ]
    # Right eye
    right_eye = [
        (cx + s(36), eye_y - s(6)),
        (cx + s(12), eye_y),
        (cx + s(20), eye_y + s(10)),
        (cx + s(38), eye_y + s(4))
    ]
    
    draw.polygon(left_eye, fill=(0, 220, 255, 255))
    draw.polygon(right_eye, fill=(0, 220, 255, 255))
    
    # Inner glowing eye core
    draw.line([(cx - s(32), eye_y - s(1)), (cx - s(16), eye_y + s(3))], fill=(255, 255, 255, 255), width=max(1, int(s(2))))
    draw.line([(cx + s(32), eye_y - s(1)), (cx + s(16), eye_y + s(3))], fill=(255, 255, 255, 255), width=max(1, int(s(2))))
    
    # 4. HUD Reticle / Tech Accents
    # Horizontal tech lines & corner brackets
    reticle_color = (0, 200, 255, 180)
    lw = max(1, int(s(2)))
    
    # Top-left HUD corner mark
    m_pad = pad + s(14)
    m_len = s(16)
    draw.line([(m_pad, m_pad), (m_pad + m_len, m_pad)], fill=reticle_color, width=lw)
    draw.line([(m_pad, m_pad), (m_pad, m_pad + m_len)], fill=reticle_color, width=lw)
    
    # Top-right HUD corner mark
    draw.line([(size - m_pad, m_pad), (size - m_pad - m_len, m_pad)], fill=reticle_color, width=lw)
    draw.line([(size - m_pad, m_pad), (size - m_pad, m_pad + m_len)], fill=reticle_color, width=lw)
    
    # Bottom-left HUD corner mark
    draw.line([(m_pad, size - m_pad), (m_pad + m_len, size - m_pad)], fill=reticle_color, width=lw)
    draw.line([(m_pad, size - m_pad), (m_pad, size - m_pad - m_len)], fill=reticle_color, width=lw)
    
    # Bottom-right HUD corner mark
    draw.line([(size - m_pad, size - m_pad), (size - m_pad - m_len, size - m_pad)], fill=reticle_color, width=lw)
    draw.line([(size - m_pad, size - m_pad), (size - m_pad, size - m_pad - m_len)], fill=reticle_color, width=lw)
    
    # Tiny HUD data dot
    dot_r = max(1, int(s(3)))
    draw.ellipse([cx - dot_r, cy + s(62) - dot_r, cx + dot_r, cy + s(62) + dot_r], fill=cyan_glow)
    
    return img

sizes = [256, 128, 64, 48, 32, 16]
images = [create_ghost_icon(s) for s in sizes]

# Save as multi-resolution ICO
images[0].save('ghosthud.ico', format='ICO', sizes=[(s, s) for s in sizes])
print("Successfully generated ghosthud.ico with sizes:", sizes)
