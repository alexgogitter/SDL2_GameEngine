from __future__ import annotations

from dataclasses import dataclass
import math
from pathlib import Path
import random
from xml.sax.saxutils import escape

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parents[1]
OUT_ROOT = ROOT / "res" / "textures" / "scifi_base"
TILE = 128
ANGLE = 0
ORTHO_WALL_ANGLES = (0, 45, 90, 135, 180, 225, 270, 315)
ORTHO_PITCH_RADIANS = math.radians(60)
FRAME_NUMBERS = {
    1: [1],
    4: [1, 3, 5, 7],
}
MAP_ORDER = (
    "Alpha",
    "Diffuse",
    "Emission",
    "Gloss",
    "Height",
    "Image",
    "Normal",
)


@dataclass(frozen=True)
class AssetSpec:
    folder: str
    name: str
    kind: str
    frames: int
    seed: int
    fps: int
    note: str
    angles: tuple[int, ...] = (ANGLE,)


ASSETS = (
    AssetSpec(
        "base_floor_panel",
        "Base_Floor_Panel",
        "floor_panel",
        4,
        1001,
        6,
        "Armored sci-fi floor panel with a subtle cyan pulse.",
    ),
    AssetSpec(
        "base_floor_grate",
        "Base_Floor_Grate",
        "floor_grate",
        4,
        1002,
        6,
        "Industrial floor grate with amber service lights.",
    ),
    AssetSpec(
        "base_wall_panel",
        "Base_Wall_Panel",
        "wall_panel",
        4,
        1003,
        5,
        "Reinforced wall segment with pulsing status indicators.",
    ),
    AssetSpec(
        "base_wall_corner",
        "Base_Wall_Corner",
        "wall_corner",
        4,
        1004,
        5,
        "L-shaped wall corner with a cyan conduit.",
    ),
    AssetSpec(
        "base_ortho_wall_panel",
        "Base_Ortho_Wall_Panel",
        "ortho_wall_panel",
        4,
        1011,
        5,
        "Orthographic 60-degree sci-fi wall segment with eight camera-yaw angles.",
        ORTHO_WALL_ANGLES,
    ),
    AssetSpec(
        "base_ortho_wall_corner",
        "Base_Ortho_Wall_Corner",
        "ortho_wall_corner",
        4,
        1012,
        5,
        "Orthographic 60-degree L-shaped sci-fi wall corner with eight camera-yaw angles.",
        ORTHO_WALL_ANGLES,
    ),
    AssetSpec(
        "ground_dust",
        "Ground_Dust",
        "ground_dust",
        1,
        2001,
        1,
        "Dusty alien ground tile.",
    ),
    AssetSpec(
        "ground_cracked",
        "Ground_Cracked",
        "ground_cracked",
        1,
        2002,
        1,
        "Cracked, compacted ground tile.",
    ),
    AssetSpec(
        "rock_cluster",
        "Rock_Cluster",
        "rock_cluster",
        1,
        3001,
        1,
        "Semi-realistic rock cluster with alpha mask.",
    ),
    AssetSpec(
        "ore_rocks",
        "Ore_Rocks",
        "ore_rocks",
        4,
        3002,
        6,
        "Rock cluster with pulsing cyan ore veins.",
    ),
    AssetSpec(
        "base_floor_xbrace",
        "Base_Floor_XBrace",
        "floor_xbrace",
        4,
        1005,
        6,
        "Heavy graphite floor panel with diagonal bracing and amber indicators.",
    ),
    AssetSpec(
        "base_floor_diamond_plate",
        "Base_Floor_Diamond_Plate",
        "floor_diamond_plate",
        1,
        1006,
        1,
        "Dark diamond-plate floor tile with reinforced corners.",
    ),
    AssetSpec(
        "base_door_frame",
        "Base_Door_Frame",
        "door_frame",
        4,
        1007,
        6,
        "Sci-fi door frame module with pulsing cyan and amber light bars.",
    ),
    AssetSpec(
        "base_pipe_corner",
        "Base_Pipe_Corner",
        "pipe_corner",
        4,
        1008,
        5,
        "Wall/floor utility pipe corner with an amber powered bend.",
    ),
    AssetSpec(
        "ground_rocky",
        "Ground_Rocky",
        "ground_rocky",
        1,
        2003,
        1,
        "Rocky dusty ground tile with embedded stones.",
    ),
    AssetSpec(
        "ground_dark_gravel",
        "Ground_Dark_Gravel",
        "ground_dark_gravel",
        1,
        2004,
        1,
        "Dark gravel ground tile for industrial or cave transitions.",
    ),
    AssetSpec(
        "ground_cracked_stone",
        "Ground_Cracked_Stone",
        "ground_cracked_stone",
        1,
        2005,
        1,
        "Grey cracked stone ground tile.",
    ),
    AssetSpec(
        "greenery_moss_patch",
        "Greenery_Moss_Patch",
        "greenery_moss_patch",
        1,
        4001,
        1,
        "Transparent moss overlay for tinting dusty ground.",
    ),
    AssetSpec(
        "greenery_grass_clumps",
        "Greenery_Grass_Clumps",
        "greenery_grass_clumps",
        1,
        4002,
        1,
        "Transparent scattered grass clump overlay.",
    ),
    AssetSpec(
        "greenery_ferns",
        "Greenery_Ferns",
        "greenery_ferns",
        1,
        4003,
        1,
        "Transparent fern and broadleaf undergrowth overlay.",
    ),
    AssetSpec(
        "greenery_vines",
        "Greenery_Vines",
        "greenery_vines",
        1,
        4004,
        1,
        "Transparent vine overlay for walls, floors, and ground.",
    ),
    AssetSpec(
        "greenery_low_bush",
        "Greenery_Low_Bush",
        "greenery_low_bush",
        1,
        4005,
        1,
        "Transparent low bush and scrub overlay.",
    ),
    AssetSpec(
        "greenery_glow_lichen",
        "Greenery_Glow_Lichen",
        "greenery_glow_lichen",
        4,
        4006,
        6,
        "Transparent alien lichen overlay with a subtle cyan emission pulse.",
    ),
)


def clamp(value: float, low: int = 0, high: int = 255) -> int:
    return max(low, min(high, int(round(value))))


def pulse(frame_index: int, frame_count: int) -> float:
    if frame_count <= 1:
        return 1.0
    return 0.35 + 0.65 * (0.5 + 0.5 * math.sin((frame_index / frame_count) * math.tau - math.pi / 2))


def rgba(color: tuple[int, int, int], alpha: int = 255) -> tuple[int, int, int, int]:
    return color[0], color[1], color[2], alpha


def gray_rgba(level: int, alpha: Image.Image | int = 255) -> Image.Image:
    image = Image.new("RGBA", (TILE, TILE), (level, level, level, 255))
    if isinstance(alpha, Image.Image):
        image.putalpha(alpha)
    else:
        image.putalpha(alpha)
    return image


def full_alpha() -> Image.Image:
    return Image.new("L", (TILE, TILE), 255)


def add_noise(image: Image.Image, seed: int, amount: int, mask: Image.Image | None = None) -> Image.Image:
    rng = np.random.default_rng(seed)
    arr = np.array(image).astype(np.int16)
    noise = rng.integers(-amount, amount + 1, size=(TILE, TILE, 1), dtype=np.int16)
    arr[:, :, :3] = np.clip(arr[:, :, :3] + noise, 0, 255)
    if mask is not None:
        mask_arr = np.array(mask)[:, :, None] / 255.0
        original = np.array(image).astype(np.int16)
        arr[:, :, :3] = (original[:, :, :3] * (1.0 - mask_arr) + arr[:, :, :3] * mask_arr).astype(np.int16)
    return Image.fromarray(arr.astype(np.uint8), "RGBA")


def add_rgb(base: Image.Image, addition: Image.Image) -> Image.Image:
    return ImageChops.add(base.convert("RGBA"), addition.convert("RGBA"))


def draw_bevel(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], light: tuple[int, int, int], shadow: tuple[int, int, int], width: int = 2) -> None:
    x0, y0, x1, y1 = box
    for offset in range(width):
        draw.line([(x0 + offset, y1 - offset), (x0 + offset, y0 + offset), (x1 - offset, y0 + offset)], fill=rgba(light))
        draw.line([(x1 - offset, y0 + offset), (x1 - offset, y1 - offset), (x0 + offset, y1 - offset)], fill=rgba(shadow))


def glow_line(emission: Image.Image, xy: tuple[int, int, int, int], color: tuple[int, int, int], width: int, strength: float, blur: float = 3.0) -> Image.Image:
    crisp = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))
    halo = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))
    crisp_draw = ImageDraw.Draw(crisp)
    halo_draw = ImageDraw.Draw(halo)
    scaled = tuple(clamp(channel * strength) for channel in color)
    halo_scaled = tuple(clamp(channel * strength * 0.45) for channel in color)
    halo_draw.line(xy, fill=rgba(halo_scaled), width=width + 8)
    crisp_draw.line(xy, fill=rgba(scaled), width=width)
    return add_rgb(add_rgb(emission, halo.filter(ImageFilter.GaussianBlur(blur))), crisp)


def glow_dot(emission: Image.Image, center: tuple[int, int], radius: int, color: tuple[int, int, int], strength: float) -> Image.Image:
    crisp = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))
    halo = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))
    cd = ImageDraw.Draw(crisp)
    hd = ImageDraw.Draw(halo)
    x, y = center
    scaled = tuple(clamp(channel * strength) for channel in color)
    halo_scaled = tuple(clamp(channel * strength * 0.35) for channel in color)
    hd.ellipse((x - radius * 3, y - radius * 3, x + radius * 3, y + radius * 3), fill=rgba(halo_scaled))
    cd.ellipse((x - radius, y - radius, x + radius, y + radius), fill=rgba(scaled))
    return add_rgb(add_rgb(emission, halo.filter(ImageFilter.GaussianBlur(4))), crisp)


def height_to_normal(height: Image.Image, alpha: Image.Image, strength: float = 5.0) -> Image.Image:
    h = np.array(height.convert("L")).astype(np.float32) / 255.0
    dx = np.zeros_like(h)
    dy = np.zeros_like(h)
    dx[:, 1:-1] = h[:, 2:] - h[:, :-2]
    dy[1:-1, :] = h[2:, :] - h[:-2, :]
    nx = -dx * strength
    ny = -dy * strength
    nz = np.ones_like(h)
    length = np.sqrt(nx * nx + ny * ny + nz * nz)
    rgb = np.stack(
        (
            (nx / length * 0.5 + 0.5) * 255.0,
            (ny / length * 0.5 + 0.5) * 255.0,
            (nz / length * 0.5 + 0.5) * 255.0,
        ),
        axis=2,
    )
    arr = np.zeros((TILE, TILE, 4), dtype=np.uint8)
    arr[:, :, :3] = np.clip(rgb, 0, 255).astype(np.uint8)
    arr[:, :, 3] = np.array(alpha, dtype=np.uint8)
    return Image.fromarray(arr, "RGBA")


def floor_panel(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    p = pulse(frame_index, frame_count)
    image = Image.new("RGBA", (TILE, TILE), (25, 31, 34, 255))
    height = Image.new("L", (TILE, TILE), 104)
    diffuse = gray_rgba(108)
    gloss = gray_rgba(72)
    emission = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))

    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    dd = ImageDraw.Draw(diffuse)
    gd = ImageDraw.Draw(gloss)

    d.rectangle((8, 8, 120, 120), fill=(41, 48, 50, 255), outline=(13, 17, 18, 255), width=2)
    draw_bevel(d, (8, 8, 120, 120), (74, 84, 84), (8, 11, 12), 3)
    hd.rectangle((8, 8, 120, 120), fill=165)
    dd.rectangle((8, 8, 120, 120), fill=(98, 98, 98, 255))
    gd.rectangle((8, 8, 120, 120), fill=(120, 120, 120, 255))

    panels = ((18, 18, 61, 61), (67, 18, 110, 61), (18, 67, 61, 110), (67, 67, 110, 110))
    for i, box in enumerate(panels):
        shade = 48 + i * 4
        d.rectangle(box, fill=(shade, shade + 8, shade + 9, 255), outline=(17, 22, 24, 255), width=2)
        draw_bevel(d, box, (83, 91, 92), (15, 18, 20), 2)
        hd.rectangle(box, fill=137)
        gd.rectangle((box[0] + 4, box[1] + 4, box[2] - 4, box[3] - 4), fill=(92, 92, 92, 255))

    for x in (64,):
        d.rectangle((x - 2, 14, x + 2, 114), fill=(15, 20, 22, 255))
        hd.rectangle((x - 2, 14, x + 2, 114), fill=62)
        dd.rectangle((x - 2, 14, x + 2, 114), fill=(158, 158, 158, 255))
    for y in (64,):
        d.rectangle((14, y - 2, 114, y + 2), fill=(15, 20, 22, 255))
        hd.rectangle((14, y - 2, 114, y + 2), fill=62)
        dd.rectangle((14, y - 2, 114, y + 2), fill=(158, 158, 158, 255))

    for x, y in ((16, 16), (112, 16), (16, 112), (112, 112), (64, 64)):
        d.ellipse((x - 3, y - 3, x + 3, y + 3), fill=(14, 17, 18, 255), outline=(92, 99, 96, 255))
        hd.ellipse((x - 3, y - 3, x + 3, y + 3), fill=88)
        gd.ellipse((x - 3, y - 3, x + 3, y + 3), fill=(155, 155, 155, 255))

    for xy in ((22, 64, 106, 64), (64, 22, 64, 106)):
        d.line(xy, fill=(11, 88, 96, 255), width=1)
        emission = glow_line(emission, xy, (0, 210, 255), 2, p, 2.5)

    image = add_noise(image, seed, 9)
    normal = height_to_normal(height, alpha, 5.5)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def floor_grate(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    p = pulse(frame_index, frame_count)
    image = Image.new("RGBA", (TILE, TILE), (20, 22, 24, 255))
    height = Image.new("L", (TILE, TILE), 92)
    diffuse = gray_rgba(118)
    gloss = gray_rgba(64)
    emission = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))

    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    dd = ImageDraw.Draw(diffuse)
    gd = ImageDraw.Draw(gloss)

    d.rectangle((7, 7, 121, 121), fill=(36, 39, 42, 255), outline=(7, 9, 10, 255), width=2)
    draw_bevel(d, (7, 7, 121, 121), (78, 80, 78), (8, 10, 12), 3)
    hd.rectangle((7, 7, 121, 121), fill=152)
    gd.rectangle((7, 7, 121, 121), fill=(108, 108, 108, 255))

    d.rectangle((23, 25, 105, 103), fill=(13, 15, 17, 255), outline=(58, 63, 62, 255), width=2)
    hd.rectangle((23, 25, 105, 103), fill=58)
    dd.rectangle((23, 25, 105, 103), fill=(170, 170, 170, 255))
    for x in range(30, 103, 10):
        d.rounded_rectangle((x, 28, x + 4, 100), radius=2, fill=(52, 57, 58, 255), outline=(88, 93, 91, 255))
        hd.rectangle((x, 28, x + 4, 100), fill=142)
        gd.rectangle((x, 28, x + 4, 100), fill=(150, 150, 150, 255))

    for y in (16, 112):
        d.rectangle((20, y - 3, 108, y + 3), fill=(56, 49, 33, 255))
        emission = glow_line(emission, (26, y, 102, y), (255, 142, 34), 3, p * 0.9, 3.0)
    for x in (16, 112):
        d.rectangle((x - 3, 22, x + 3, 106), fill=(55, 47, 33, 255))
        emission = glow_line(emission, (x, 28, x, 100), (255, 142, 34), 3, p * 0.65, 3.0)

    image = add_noise(image, seed, 7)
    normal = height_to_normal(height, alpha, 6.0)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def wall_panel(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    p = pulse(frame_index, frame_count)
    image = Image.new("RGBA", (TILE, TILE), (23, 26, 28, 255))
    height = Image.new("L", (TILE, TILE), 92)
    diffuse = gray_rgba(104)
    gloss = gray_rgba(75)
    emission = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))

    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    dd = ImageDraw.Draw(diffuse)
    gd = ImageDraw.Draw(gloss)

    d.rectangle((8, 14, 120, 100), fill=(48, 53, 56, 255), outline=(10, 12, 14, 255), width=3)
    draw_bevel(d, (8, 14, 120, 100), (92, 96, 94), (7, 9, 11), 3)
    hd.rectangle((8, 14, 120, 100), fill=190)
    gd.rectangle((8, 14, 120, 100), fill=(130, 130, 130, 255))

    d.rectangle((16, 26, 78, 88), fill=(35, 40, 43, 255), outline=(13, 16, 18, 255), width=2)
    draw_bevel(d, (16, 26, 78, 88), (75, 80, 80), (12, 14, 16), 2)
    hd.rectangle((16, 26, 78, 88), fill=148)
    d.rectangle((86, 22, 109, 92), fill=(28, 34, 38, 255), outline=(9, 12, 14, 255), width=2)
    hd.rectangle((86, 22, 109, 92), fill=126)
    dd.rectangle((86, 22, 109, 92), fill=(142, 142, 142, 255))

    emission = glow_line(emission, (97, 28, 97, 86), (0, 210, 255), 3, p, 3.0)
    d.line((97, 28, 97, 86), fill=(10, 91, 102, 255), width=2)
    for index, y in enumerate((36, 56, 76)):
        color = (255, 59, 44) if index == frame_index % 3 else (0, 180, 220)
        strength = 0.8 if index == frame_index % 3 else 0.35
        emission = glow_dot(emission, (48 + index * 9, y), 3, color, strength)
        d.ellipse((45 + index * 9, y - 3, 51 + index * 9, y + 3), fill=rgba(tuple(clamp(c * 0.45) for c in color)))

    d.rectangle((6, 100, 122, 118), fill=(13, 15, 17, 255))
    image = add_noise(image, seed, 8)
    normal = height_to_normal(height, alpha, 5.0)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def wall_corner(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    p = pulse(frame_index, frame_count)
    image = Image.new("RGBA", (TILE, TILE), (28, 32, 35, 255))
    height = Image.new("L", (TILE, TILE), 96)
    diffuse = gray_rgba(112)
    gloss = gray_rgba(68)
    emission = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))

    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    gd = ImageDraw.Draw(gloss)

    d.rectangle((7, 7, 121, 42), fill=(47, 53, 56, 255), outline=(9, 11, 12, 255), width=2)
    d.rectangle((7, 7, 42, 121), fill=(47, 53, 56, 255), outline=(9, 11, 12, 255), width=2)
    draw_bevel(d, (7, 7, 121, 42), (92, 96, 93), (8, 10, 11), 3)
    draw_bevel(d, (7, 7, 42, 121), (92, 96, 93), (8, 10, 11), 3)
    hd.rectangle((7, 7, 121, 42), fill=188)
    hd.rectangle((7, 7, 42, 121), fill=188)
    gd.rectangle((7, 7, 121, 42), fill=(128, 128, 128, 255))
    gd.rectangle((7, 7, 42, 121), fill=(128, 128, 128, 255))

    d.rectangle((47, 47, 121, 121), fill=(34, 40, 42, 255), outline=(15, 18, 20, 255), width=2)
    draw_bevel(d, (47, 47, 121, 121), (70, 76, 76), (11, 14, 16), 2)
    hd.rectangle((47, 47, 121, 121), fill=132)
    for xy in ((20, 18, 104, 18), (20, 18, 20, 104)):
        d.line(xy, fill=(10, 92, 104, 255), width=2)
        emission = glow_line(emission, xy, (0, 210, 255), 3, p * 0.9, 3.0)
    emission = glow_dot(emission, (20, 18), 4, (0, 210, 255), p)

    image = add_noise(image, seed, 8)
    normal = height_to_normal(height, alpha, 5.0)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def ground_dust(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    alpha = full_alpha()
    image = Image.new("RGBA", (TILE, TILE), (89, 83, 72, 255))
    height = Image.new("L", (TILE, TILE), 104)
    diffuse = gray_rgba(205)
    gloss = gray_rgba(18)
    emission = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for _ in range(90):
        x = rng.randint(0, TILE - 1)
        y = rng.randint(0, TILE - 1)
        r = rng.randint(1, 4)
        shade = rng.randint(-20, 22)
        color = (clamp(89 + shade), clamp(83 + shade), clamp(72 + shade * 0.8), 255)
        d.ellipse((x - r, y - r, x + r, y + r), fill=color)
        hd.ellipse((x - r, y - r, x + r, y + r), fill=clamp(104 + shade))
    for _ in range(12):
        x = rng.randint(3, 124)
        y = rng.randint(3, 124)
        r = rng.randint(1, 3)
        d.ellipse((x - r, y - r, x + r, y + r), fill=(61, 59, 54, 255))
        hd.ellipse((x - r, y - r, x + r, y + r), fill=135)
    image = add_noise(image, seed + 55, 10)
    normal = height_to_normal(height, alpha, 2.8)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def ground_cracked(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    maps = ground_dust(frame_index, frame_count, seed)
    image = maps["Image"].copy()
    height = maps["Height"].convert("L")
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    starts = [(12, 23), (96, 18), (65, 104), (20, 87)]
    for sx, sy in starts:
        points = [(sx, sy)]
        x, y = sx, sy
        for _ in range(rng.randint(4, 7)):
            x += rng.randint(-18, 20)
            y += rng.randint(9, 21)
            points.append((clamp(x, -20, 148), clamp(y, -20, 148)))
        d.line(points, fill=(30, 29, 28, 255), width=3, joint="curve")
        d.line(points, fill=(73, 64, 54, 255), width=1)
        hd.line(points, fill=34, width=4)
    maps["Image"] = image
    maps["Height"] = gray_rgba(0)
    maps["Height"].putalpha(255)
    maps["Height"] = Image.merge("RGBA", (height, height, height, full_alpha()))
    maps["Normal"] = height_to_normal(height, full_alpha(), 3.6)
    return maps


def rock_polygons(seed: int) -> list[list[tuple[int, int]]]:
    rng = random.Random(seed)
    centers = ((39, 73, 26), (67, 58, 32), (87, 81, 25), (58, 93, 21), (83, 43, 18))
    polygons: list[list[tuple[int, int]]] = []
    for cx, cy, radius in centers:
        points = []
        sides = rng.randint(7, 10)
        for i in range(sides):
            angle = (i / sides) * math.tau + rng.uniform(-0.18, 0.18)
            local_radius = radius * rng.uniform(0.68, 1.08)
            points.append((round(cx + math.cos(angle) * local_radius), round(cy + math.sin(angle) * local_radius * 0.78)))
        polygons.append(points)
    return polygons


def rock_cluster_base(seed: int, ore: bool, frame_index: int, frame_count: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    diffuse = Image.new("RGBA", (TILE, TILE), (220, 220, 220, 0))
    gloss = Image.new("RGBA", (TILE, TILE), (20, 20, 20, 0))
    emission = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 255))

    ad = ImageDraw.Draw(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    dd = ImageDraw.Draw(diffuse)
    gd = ImageDraw.Draw(gloss)
    polygons = rock_polygons(seed)
    for index, polygon in enumerate(polygons):
        shade = 82 + index * 9 + rng.randint(-6, 6)
        fill = (shade, shade + 5, shade + 8, 255)
        outline = (34, 37, 39, 255)
        ad.polygon(polygon, fill=255)
        d.polygon(polygon, fill=fill, outline=outline)
        hd.polygon(polygon, fill=135 + index * 12)
        dd.polygon(polygon, fill=(210, 210, 210, 255))
        gd.polygon(polygon, fill=(25 + index * 2, 25 + index * 2, 25 + index * 2, 255))
        for _ in range(3):
            p0 = rng.choice(polygon)
            p1 = rng.choice(polygon)
            d.line((p0, p1), fill=(clamp(shade + 35), clamp(shade + 36), clamp(shade + 38), 255), width=1)

    alpha = alpha.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.GaussianBlur(0.35))
    image.putalpha(alpha)
    diffuse.putalpha(alpha)
    gloss.putalpha(alpha)

    if ore:
        p = pulse(frame_index, frame_count)
        vein_color = (0, 220, 255)
        vein_paths = (
            ((48, 65), (61, 57), (76, 61), (89, 52)),
            ((58, 86), (69, 76), (83, 83), (96, 76)),
            ((37, 74), (50, 78), (65, 70)),
        )
        for path in vein_paths:
            d.line(path, fill=(18, 126, 142, 255), width=2)
            for a, b in zip(path, path[1:]):
                emission = glow_line(emission, (a[0], a[1], b[0], b[1]), vein_color, 3, p, 3.0)
                gd.line((a, b), fill=(160, 195, 205, 255), width=2)
                hd.line((a, b), fill=182, width=2)

    normal = height_to_normal(gray_rgba_from_l(height, alpha), alpha, 4.2)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def gray_rgba_from_l(image_l: Image.Image, alpha: Image.Image) -> Image.Image:
    return Image.merge("RGBA", (image_l, image_l, image_l, alpha))


def material_maps(
    image: Image.Image,
    alpha: Image.Image,
    diffuse: Image.Image,
    gloss: Image.Image,
    height: Image.Image,
    normal: Image.Image,
    emission: Image.Image,
) -> dict[str, Image.Image]:
    image_rgba = image.convert("RGBA")
    image_rgba.putalpha(alpha)
    diffuse_rgba = diffuse.convert("RGBA")
    diffuse_rgba.putalpha(alpha)
    gloss_rgba = gloss.convert("RGBA")
    gloss_rgba.putalpha(alpha)
    emission_rgba = emission.convert("RGBA")
    emission_rgba.putalpha(alpha)
    if height.mode == "L":
        height_rgba = gray_rgba_from_l(height, alpha)
    else:
        height_rgba = height.convert("RGBA")
        height_rgba.putalpha(alpha)
    normal_rgba = normal.convert("RGBA")
    normal_rgba.putalpha(alpha)
    alpha_rgba = gray_rgba_from_l(alpha, alpha)
    return {
        "Alpha": alpha_rgba,
        "Diffuse": diffuse_rgba,
        "Emission": emission_rgba,
        "Gloss": gloss_rgba,
        "Height": height_rgba,
        "Image": image_rgba,
        "Normal": normal_rgba,
    }


def refined_alpha(padding: int = 4, radius: int = 5) -> Image.Image:
    alpha = Image.new("L", (TILE, TILE), 0)
    ImageDraw.Draw(alpha).rounded_rectangle(
        (padding, padding, TILE - padding - 1, TILE - padding - 1),
        radius=radius,
        fill=255,
    )
    return alpha


def subtract_mask(base: Image.Image, box: tuple[int, int, int, int], radius: int = 4) -> Image.Image:
    cutout = Image.new("L", (TILE, TILE), 0)
    ImageDraw.Draw(cutout).rounded_rectangle(box, radius=radius, fill=255)
    return ImageChops.subtract(base, cutout)


def black_map(alpha: Image.Image) -> Image.Image:
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    image.putalpha(alpha)
    return image


def grunge(image: Image.Image, seed: int, alpha: Image.Image, amount: int = 18, blur: float = 0.8) -> Image.Image:
    rng = np.random.default_rng(seed)
    noise = rng.integers(-amount, amount + 1, size=(TILE, TILE), dtype=np.int16)
    broad = Image.fromarray(np.uint8(np.clip(noise + 128, 0, 255)), "L").filter(
        ImageFilter.GaussianBlur(blur)
    )
    broad_arr = np.array(broad).astype(np.int16) - 128
    arr = np.array(image).astype(np.int16)
    arr[:, :, :3] = np.clip(arr[:, :, :3] + broad_arr[:, :, None], 0, 255)
    arr[:, :, 3] = np.array(alpha)
    return Image.fromarray(arr.astype(np.uint8), "RGBA")


def refined_base(
    alpha: Image.Image,
    seed: int,
    base: tuple[int, int, int] = (37, 40, 41),
    amount: int = 16,
) -> Image.Image:
    image = Image.new("RGBA", (TILE, TILE), rgba(base, 255))
    return grunge(image, seed, alpha, amount)


def draw_plate(
    image: Image.Image,
    height: Image.Image,
    box: tuple[int, int, int, int],
    fill: tuple[int, int, int],
    rim: tuple[int, int, int] = (15, 17, 18),
    raised: int = 158,
) -> None:
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.rounded_rectangle(box, radius=4, fill=rgba(fill), outline=rgba(rim), width=2)
    draw_bevel(d, box, (82, 86, 84), (8, 10, 11), 2)
    hd.rounded_rectangle(box, radius=4, fill=raised)
    inset = (box[0] + 5, box[1] + 5, box[2] - 5, box[3] - 5)
    d.rounded_rectangle(inset, radius=3, outline=(21, 23, 24, 255), width=1)
    hd.rounded_rectangle(inset, radius=3, outline=raised - 28, width=1)


def draw_bolt(image: Image.Image, height: Image.Image, x: int, y: int, r: int = 3) -> None:
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.ellipse((x - r, y - r, x + r, y + r), fill=(11, 12, 13, 255), outline=(90, 93, 91, 255))
    d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=(42, 44, 44, 255))
    hd.ellipse((x - r, y - r, x + r, y + r), fill=184)


def draw_corner_brackets(image: Image.Image, height: Image.Image, pad: int = 10) -> None:
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    length = 18
    pieces = (
        ((pad, pad), (pad + length, pad), (pad, pad + length)),
        ((TILE - pad, pad), (TILE - pad - length, pad), (TILE - pad, pad + length)),
        ((pad, TILE - pad), (pad + length, TILE - pad), (pad, TILE - pad - length)),
        ((TILE - pad, TILE - pad), (TILE - pad - length, TILE - pad), (TILE - pad, TILE - pad - length)),
    )
    for corner, a, b in pieces:
        d.line((a, corner, b), fill=(94, 99, 98, 255), width=3)
        d.line((a, corner, b), fill=(16, 18, 19, 255), width=1)
        hd.line((a, corner, b), fill=190, width=3)


def draw_scratches(image: Image.Image, seed: int, count: int = 22) -> None:
    rng = random.Random(seed)
    d = ImageDraw.Draw(image)
    for _ in range(count):
        x = rng.randint(10, 118)
        y = rng.randint(10, 118)
        length = rng.randint(5, 18)
        angle = rng.uniform(-0.6, 0.6)
        x2 = round(x + math.cos(angle) * length)
        y2 = round(y + math.sin(angle) * length)
        shade = rng.randint(58, 92)
        d.line((x, y, x2, y2), fill=(shade, shade, shade, 110), width=1)


def draw_light_bar(
    image: Image.Image,
    emission: Image.Image,
    box: tuple[int, int, int, int],
    color: tuple[int, int, int],
    strength: float,
) -> Image.Image:
    d = ImageDraw.Draw(image)
    x0, y0, x1, y1 = box
    d.rounded_rectangle((x0 - 2, y0 - 2, x1 + 2, y1 + 2), radius=3, fill=(23, 17, 9, 255))
    d.rounded_rectangle(box, radius=2, fill=rgba(tuple(clamp(c * 0.55) for c in color)))
    glow = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    gd = ImageDraw.Draw(glow)
    scaled = tuple(clamp(c * strength) for c in color)
    gd.rounded_rectangle(box, radius=2, fill=rgba(scaled))
    halo = glow.filter(ImageFilter.GaussianBlur(4))
    return add_rgb(add_rgb(emission, halo), glow)


def draw_vent(image: Image.Image, height: Image.Image, box: tuple[int, int, int, int], horizontal: bool = True) -> None:
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.rounded_rectangle(box, radius=3, fill=(14, 15, 16, 255), outline=(65, 68, 67, 255), width=2)
    hd.rounded_rectangle(box, radius=3, fill=70)
    if horizontal:
        for y in range(box[1] + 6, box[3] - 3, 6):
            d.line((box[0] + 5, y, box[2] - 5, y), fill=(74, 77, 75, 255), width=2)
            hd.line((box[0] + 5, y, box[2] - 5, y), fill=132, width=2)
    else:
        for x in range(box[0] + 6, box[2] - 3, 6):
            d.line((x, box[1] + 5, x, box[3] - 5), fill=(74, 77, 75, 255), width=2)
            hd.line((x, box[1] + 5, x, box[3] - 5), fill=132, width=2)


def panel_maps(
    image: Image.Image,
    alpha: Image.Image,
    height: Image.Image,
    emission: Image.Image,
    roughness: int = 96,
    gloss_level: int = 92,
    normal_strength: float = 5.5,
) -> dict[str, Image.Image]:
    diffuse = gray_rgba(roughness, alpha)
    gloss = gray_rgba(gloss_level, alpha)
    normal = height_to_normal(height, alpha, normal_strength)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def floor_panel(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 5)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (34, 37, 38), 18)
    height = Image.new("L", (TILE, TILE), 82)
    emission = black_map(alpha)
    draw_plate(image, height, (7, 7, 120, 120), (42, 45, 45), raised=154)
    for box in ((16, 16, 59, 59), (69, 16, 112, 59), (16, 69, 59, 112), (69, 69, 112, 112)):
        draw_plate(image, height, box, (47, 50, 50), raised=130)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for xy in ((64, 18, 64, 110), (18, 64, 110, 64)):
        d.line(xy, fill=(9, 11, 12, 255), width=5)
        d.line(xy, fill=(72, 77, 75, 255), width=1)
        hd.line(xy, fill=48, width=5)
    emission = draw_light_bar(image, emission, (59, 60, 69, 68), (255, 143, 28), p)
    for x, y in ((15, 15), (113, 15), (15, 113), (113, 113), (64, 64)):
        draw_bolt(image, height, x, y, 3)
    draw_corner_brackets(image, height, 12)
    draw_scratches(image, seed + 23, 24)
    return panel_maps(image, alpha, height, emission, roughness=105, gloss_level=88)


def floor_grate(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 5)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (32, 34, 35), 14)
    height = Image.new("L", (TILE, TILE), 76)
    emission = black_map(alpha)
    draw_plate(image, height, (7, 7, 120, 120), (39, 41, 41), raised=148)
    draw_vent(image, height, (22, 26, 106, 100), horizontal=False)
    for x in (17, 111):
        emission = draw_light_bar(image, emission, (x - 2, 29, x + 2, 98), (255, 143, 28), p * 0.85)
    for y in (17, 111):
        d = ImageDraw.Draw(image)
        d.line((29, y, 99, y), fill=(91, 82, 54, 255), width=2)
        emission = glow_line(emission, (29, y, 99, y), (255, 143, 28), 2, p * 0.5, 2.0)
    for x, y in ((16, 16), (112, 16), (16, 112), (112, 112)):
        draw_bolt(image, height, x, y, 3)
    draw_scratches(image, seed + 12, 18)
    return panel_maps(image, alpha, height, emission, roughness=130, gloss_level=95)


def floor_xbrace(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 5)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (35, 37, 38), 16)
    height = Image.new("L", (TILE, TILE), 88)
    emission = black_map(alpha)
    draw_plate(image, height, (7, 7, 120, 120), (42, 44, 44), raised=146)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for xy in ((24, 24, 104, 104), (104, 24, 24, 104)):
        d.line(xy, fill=(10, 11, 12, 255), width=15)
        d.line(xy, fill=(74, 78, 77, 255), width=11)
        d.line(xy, fill=(23, 25, 26, 255), width=5)
        hd.line(xy, fill=185, width=12)
    d.rounded_rectangle((45, 45, 83, 83), radius=4, fill=(31, 33, 34, 255), outline=(91, 95, 93, 255), width=2)
    hd.rounded_rectangle((45, 45, 83, 83), radius=4, fill=150)
    for box in ((15, 58, 24, 66), (104, 58, 113, 66), (58, 15, 66, 24), (58, 104, 66, 113)):
        emission = draw_light_bar(image, emission, box, (255, 143, 28), p)
    draw_corner_brackets(image, height, 12)
    draw_scratches(image, seed + 37, 18)
    return panel_maps(image, alpha, height, emission, roughness=108, gloss_level=96)


def floor_diamond_plate(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 5)
    image = refined_base(alpha, seed, (34, 36, 36), 15)
    height = Image.new("L", (TILE, TILE), 95)
    emission = black_map(alpha)
    draw_plate(image, height, (7, 7, 120, 120), (38, 41, 41), raised=145)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for x in range(-24, 150, 12):
        d.line((x, 30, x + 66, 96), fill=(66, 69, 67, 255), width=1)
        d.line((x + 66, 30, x, 96), fill=(15, 16, 17, 255), width=1)
        hd.line((x, 30, x + 66, 96), fill=160, width=1)
        hd.line((x + 66, 30, x, 96), fill=65, width=1)
    draw_corner_brackets(image, height, 13)
    draw_scratches(image, seed + 19, 30)
    return panel_maps(image, alpha, height, emission, roughness=120, gloss_level=78)


def wall_panel(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 4)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (33, 35, 36), 18)
    height = Image.new("L", (TILE, TILE), 76)
    emission = black_map(alpha)
    draw_plate(image, height, (6, 8, 121, 120), (43, 45, 45), raised=144)
    for box in ((14, 16, 48, 58), (51, 16, 85, 58), (88, 16, 114, 58), (14, 64, 48, 111), (51, 64, 85, 111)):
        draw_plate(image, height, box, (48, 50, 50), raised=128)
    draw_vent(image, height, (91, 68, 113, 107), horizontal=True)
    emission = draw_light_bar(image, emission, (96, 22, 102, 56), (255, 143, 28), p)
    for y in (28, 40, 52):
        emission = draw_light_bar(image, emission, (105, y, 109, y + 4), (255, 143, 28), p * 0.85)
    for x, y in ((14, 15), (114, 15), (14, 113), (114, 113), (50, 62), (88, 62)):
        draw_bolt(image, height, x, y, 2)
    draw_scratches(image, seed + 44, 20)
    return panel_maps(image, alpha, height, emission, roughness=112, gloss_level=84)


def wall_corner(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 4)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (32, 35, 36), 16)
    height = Image.new("L", (TILE, TILE), 72)
    emission = black_map(alpha)
    draw_plate(image, height, (7, 7, 121, 39), (45, 48, 48), raised=154)
    draw_plate(image, height, (7, 7, 39, 121), (45, 48, 48), raised=154)
    draw_plate(image, height, (43, 43, 121, 121), (38, 41, 42), raised=125)
    emission = draw_light_bar(image, emission, (23, 16, 90, 20), (0, 217, 255), p)
    emission = draw_light_bar(image, emission, (16, 23, 20, 90), (0, 217, 255), p)
    for x, y in ((16, 16), (112, 16), (16, 112), (112, 112), (40, 40)):
        draw_bolt(image, height, x, y, 2)
    draw_scratches(image, seed + 45, 16)
    return panel_maps(image, alpha, height, emission, roughness=112, gloss_level=86)


def rotate_ground(x: float, z: float, degrees: float) -> tuple[float, float]:
    radians = math.radians(degrees)
    c = math.cos(radians)
    s = math.sin(radians)
    return x * c - z * s, x * s + z * c


def project_ortho60(
    x: float,
    y: float,
    z: float,
    view_angle: int,
    scale: float = 1.12,
    center: tuple[float, float] = (64.0, 82.0),
) -> tuple[int, int]:
    rx, rz = rotate_ground(x, z, view_angle)
    screen_x = center[0] + rx * scale
    screen_y = center[1] + (rz * math.cos(ORTHO_PITCH_RADIANS) - y * math.sin(ORTHO_PITCH_RADIANS)) * scale
    return round(screen_x), round(screen_y)


def projected_depth(points: list[tuple[float, float, float]], view_angle: int) -> float:
    return sum(rotate_ground(point[0], point[2], view_angle)[1] for point in points) / len(points)


def prism_point(
    center_x: float,
    center_z: float,
    axis_angle: float,
    u: float,
    p: float,
    y: float,
) -> tuple[float, float, float]:
    radians = math.radians(axis_angle)
    axis = (math.cos(radians), math.sin(radians))
    perp = (-math.sin(radians), math.cos(radians))
    return (
        center_x + axis[0] * u + perp[0] * p,
        y,
        center_z + axis[1] * u + perp[1] * p,
    )


def prism_faces(
    center_x: float,
    center_z: float,
    axis_angle: float,
    length: float,
    thickness: float,
    wall_height: float,
    view_angle: int,
) -> list[dict[str, object]]:
    half_l = length * 0.5
    half_t = thickness * 0.5
    faces: list[dict[str, object]] = []

    def point(u: float, p: float, y: float) -> tuple[float, float, float]:
        return prism_point(center_x, center_z, axis_angle, u, p, y)

    for side in (-1.0, 1.0):
        p = side * half_t
        points = [
            point(-half_l, p, 0.0),
            point(half_l, p, 0.0),
            point(half_l, p, wall_height),
            point(-half_l, p, wall_height),
        ]
        normal = rotate_ground(-math.sin(math.radians(axis_angle)) * side, math.cos(math.radians(axis_angle)) * side, view_angle)[1]
        faces.append({"kind": "long", "points": points, "depth": projected_depth(points, view_angle), "normal": normal})

    for side in (-1.0, 1.0):
        u = side * half_l
        points = [
            point(u, -half_t, 0.0),
            point(u, half_t, 0.0),
            point(u, half_t, wall_height),
            point(u, -half_t, wall_height),
        ]
        normal = rotate_ground(math.cos(math.radians(axis_angle)) * side, math.sin(math.radians(axis_angle)) * side, view_angle)[1]
        faces.append({"kind": "end", "points": points, "depth": projected_depth(points, view_angle), "normal": normal})

    points = [
        point(-half_l, -half_t, wall_height),
        point(half_l, -half_t, wall_height),
        point(half_l, half_t, wall_height),
        point(-half_l, half_t, wall_height),
    ]
    faces.append({"kind": "top", "points": points, "depth": projected_depth(points, view_angle) + 999.0, "normal": 0.0})
    return faces


def draw_ortho_prisms(
    image: Image.Image,
    height: Image.Image,
    alpha: Image.Image,
    view_angle: int,
    prisms: list[tuple[float, float, float, float, float, float]],
) -> None:
    records: list[dict[str, object]] = []
    for prism in prisms:
        records.extend(prism_faces(*prism, view_angle))

    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    ad = ImageDraw.Draw(alpha)
    for face in sorted(records, key=lambda item: float(item["depth"])):
        points_3d = face["points"]
        points_2d = [project_ortho60(x, y, z, view_angle) for x, y, z in points_3d]  # type: ignore[misc]
        normal = float(face["normal"])
        if face["kind"] == "top":
            base = (56, 60, 60)
            raised = 214
        elif face["kind"] == "long":
            base = (39, 42, 42)
            raised = 142 + clamp(normal * 34, -12, 34)
        else:
            base = (32, 35, 36)
            raised = 118 + clamp(normal * 28, -10, 26)
        light = 0.82 + max(normal, 0.0) * 0.23
        fill = tuple(clamp(channel * light) for channel in base)
        ad.polygon(points_2d, fill=255)
        d.polygon(points_2d, fill=rgba(fill), outline=(9, 11, 12, 255))
        hd.polygon(points_2d, fill=raised)
        d.line(points_2d + [points_2d[0]], fill=(83, 88, 86, 180), width=1)


def visible_side_sign(axis_angle: float, view_angle: int) -> float:
    radians = math.radians(axis_angle)
    perp = (-math.sin(radians), math.cos(radians))
    _, rotated_z = rotate_ground(perp[0], perp[1], view_angle)
    return 1.0 if rotated_z >= 0.0 else -1.0


def surface_poly(
    center_x: float,
    center_z: float,
    axis_angle: float,
    thickness: float,
    view_angle: int,
    u0: float,
    u1: float,
    y0: float,
    y1: float,
    side: float,
) -> list[tuple[int, int]]:
    p = side * thickness * 0.5
    points = (
        prism_point(center_x, center_z, axis_angle, u0, p, y0),
        prism_point(center_x, center_z, axis_angle, u1, p, y0),
        prism_point(center_x, center_z, axis_angle, u1, p, y1),
        prism_point(center_x, center_z, axis_angle, u0, p, y1),
    )
    return [project_ortho60(x, y, z, view_angle) for x, y, z in points]


def surface_point(
    center_x: float,
    center_z: float,
    axis_angle: float,
    thickness: float,
    view_angle: int,
    u: float,
    y: float,
    side: float,
) -> tuple[int, int]:
    x, world_y, z = prism_point(center_x, center_z, axis_angle, u, side * thickness * 0.5, y)
    return project_ortho60(x, world_y, z, view_angle)


def draw_surface_plate(
    image: Image.Image,
    height: Image.Image,
    view_angle: int,
    center_x: float,
    center_z: float,
    axis_angle: float,
    length: float,
    thickness: float,
    u0: float,
    u1: float,
    y0: float,
    y1: float,
    side: float,
    fill: tuple[int, int, int],
    raised: int,
) -> None:
    points = surface_poly(center_x, center_z, axis_angle, thickness, view_angle, u0, u1, y0, y1, side)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.polygon(points, fill=rgba(fill), outline=(13, 15, 16, 255))
    hd.polygon(points, fill=raised)
    inset_u = max(2.0, length * 0.025)
    inset_y = 3.0
    inner = surface_poly(
        center_x,
        center_z,
        axis_angle,
        thickness,
        view_angle,
        u0 + inset_u,
        u1 - inset_u,
        y0 + inset_y,
        y1 - inset_y,
        side,
    )
    d.line(inner + [inner[0]], fill=(74, 79, 77, 170), width=1)


def draw_projected_light_line(
    image: Image.Image,
    emission: Image.Image,
    start: tuple[int, int],
    end: tuple[int, int],
    color: tuple[int, int, int],
    strength: float,
    width: int = 3,
) -> Image.Image:
    d = ImageDraw.Draw(image)
    d.line((start[0], start[1], end[0], end[1]), fill=rgba(tuple(clamp(c * 0.45) for c in color)), width=width)
    return glow_line(emission, (start[0], start[1], end[0], end[1]), color, width, strength, 3.0)


def draw_ortho_wall_details(
    image: Image.Image,
    height: Image.Image,
    emission: Image.Image,
    view_angle: int,
    frame_pulse: float,
    center_x: float,
    center_z: float,
    axis_angle: float,
    length: float,
    thickness: float,
    wall_height: float,
) -> Image.Image:
    side = visible_side_sign(axis_angle, view_angle)
    panel_y0 = 10.0
    panel_y1 = wall_height - 13.0
    gap = 4.0
    usable = length - 24.0
    panel_w = (usable - gap * 2.0) / 3.0
    u = -length * 0.5 + 12.0
    for index in range(3):
        draw_surface_plate(
            image,
            height,
            view_angle,
            center_x,
            center_z,
            axis_angle,
            length,
            thickness,
            u,
            u + panel_w,
            panel_y0,
            panel_y1,
            side,
            (45 + index * 2, 48 + index * 2, 48 + index * 2),
            176,
        )
        u += panel_w + gap

    cyan_start = surface_point(center_x, center_z, axis_angle, thickness, view_angle, -length * 0.5 + 14.0, wall_height - 9.0, side)
    cyan_end = surface_point(center_x, center_z, axis_angle, thickness, view_angle, length * 0.5 - 14.0, wall_height - 9.0, side)
    emission = draw_projected_light_line(image, emission, cyan_start, cyan_end, (0, 217, 255), frame_pulse, 3)

    module_u = length * 0.5 - 18.0
    for y0 in (16.0, 27.0, 38.0):
        start = surface_point(center_x, center_z, axis_angle, thickness, view_angle, module_u, y0, side)
        end = surface_point(center_x, center_z, axis_angle, thickness, view_angle, module_u, y0 + 6.0, side)
        emission = draw_projected_light_line(image, emission, start, end, (255, 143, 28), frame_pulse * 0.85, 2)

    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for bolt_u in (-length * 0.5 + 8.0, length * 0.5 - 8.0):
        for bolt_y in (8.0, wall_height - 8.0):
            x, y = surface_point(center_x, center_z, axis_angle, thickness, view_angle, bolt_u, bolt_y, side)
            d.ellipse((x - 2, y - 2, x + 2, y + 2), fill=(12, 13, 14, 255), outline=(91, 96, 94, 255))
            hd.ellipse((x - 2, y - 2, x + 2, y + 2), fill=206)
    return emission


def ortho_wall_panel(frame_index: int, frame_count: int, seed: int, view_angle: int) -> dict[str, Image.Image]:
    p = pulse(frame_index, frame_count)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    length = 88.0
    thickness = 17.0
    wall_height = 54.0
    prism = (0.0, 0.0, 0.0, length, thickness, wall_height)
    draw_ortho_prisms(image, height, alpha, view_angle, [prism])
    image = grunge(image, seed, alpha, 10, 0.7)
    draw_scratches(image, seed + view_angle + 61, 15)
    emission = draw_ortho_wall_details(image, height, emission, view_angle, p, *prism)
    return panel_maps(image, alpha, height, emission, roughness=105, gloss_level=92, normal_strength=5.8)


def ortho_wall_corner(frame_index: int, frame_count: int, seed: int, view_angle: int) -> dict[str, Image.Image]:
    p = pulse(frame_index, frame_count)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    length = 70.0
    thickness = 17.0
    wall_height = 54.0
    prisms = [
        (length * 0.25, 0.0, 0.0, length, thickness, wall_height),
        (0.0, length * 0.25, 90.0, length, thickness, wall_height),
    ]
    draw_ortho_prisms(image, height, alpha, view_angle, prisms)
    image = grunge(image, seed, alpha, 10, 0.7)
    draw_scratches(image, seed + view_angle + 73, 12)
    for prism in prisms:
        emission = draw_ortho_wall_details(image, height, emission, view_angle, p * 0.9, *prism)
    joint_a = project_ortho60(0.0, wall_height - 7.0, 0.0, view_angle)
    joint_b = project_ortho60(12.0, wall_height - 7.0, 12.0, view_angle)
    emission = draw_projected_light_line(image, emission, joint_a, joint_b, (0, 217, 255), p, 3)
    return panel_maps(image, alpha, height, emission, roughness=108, gloss_level=94, normal_strength=5.8)


def door_frame(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    outer = refined_alpha(5, 6)
    alpha = subtract_mask(outer, (32, 32, 96, 112), 5)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (32, 34, 35), 12)
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    draw_plate(image, height, (8, 8, 120, 120), (40, 42, 43), raised=118)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.rounded_rectangle((28, 26, 100, 118), radius=8, fill=(12, 13, 14, 255), outline=(88, 92, 90, 255), width=4)
    d.rounded_rectangle((35, 35, 93, 112), radius=5, fill=(0, 0, 0, 0))
    hd.rounded_rectangle((28, 26, 100, 118), radius=8, fill=190)
    hd.rounded_rectangle((35, 35, 93, 112), radius=5, fill=0)
    for box, color in (
        ((43, 18, 85, 23), (0, 217, 255)),
        ((25, 50, 30, 92), (255, 143, 28)),
        ((98, 50, 103, 92), (255, 143, 28)),
    ):
        emission = draw_light_bar(image, emission, box, color, p)
    draw_corner_brackets(image, height, 13)
    return panel_maps(image, alpha, height, emission, roughness=102, gloss_level=92)


def pipe_corner(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(5, 5)
    p = pulse(frame_index, frame_count)
    image = refined_base(alpha, seed, (30, 32, 33), 13)
    height = Image.new("L", (TILE, TILE), 72)
    emission = black_map(alpha)
    draw_plate(image, height, (7, 7, 120, 120), (36, 38, 38), raised=124)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for offset, width, shade in ((0, 12, (14, 15, 16)), (4, 6, (64, 67, 67))):
        d.line((18, 48 + offset, 78, 48 + offset, 78, 96), fill=rgba(shade), width=width, joint="curve")
        hd.line((18, 48 + offset, 78, 48 + offset, 78, 96), fill=170 if width < 10 else 98, width=width)
        d.line((48 + offset, 18, 48 + offset, 78, 96, 78), fill=rgba(shade), width=width, joint="curve")
        hd.line((48 + offset, 18, 48 + offset, 78, 96, 78), fill=170 if width < 10 else 98, width=width)
    emission = draw_light_bar(image, emission, (91, 85, 103, 96), (255, 143, 28), p)
    for x, y in ((19, 48), (48, 19), (78, 96), (96, 78)):
        draw_bolt(image, height, x, y, 2)
    return panel_maps(image, alpha, height, emission, roughness=115, gloss_level=100)


def speckled_ground(seed: int, base: tuple[int, int, int], dark: tuple[int, int, int], light: tuple[int, int, int], pebble_count: int) -> tuple[Image.Image, Image.Image]:
    rng = random.Random(seed)
    alpha = full_alpha()
    image = Image.new("RGBA", (TILE, TILE), rgba(base))
    height = Image.new("L", (TILE, TILE), 104)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for _ in range(180):
        x = rng.randint(0, TILE - 1)
        y = rng.randint(0, TILE - 1)
        r = rng.choice((1, 1, 1, 2, 2, 3))
        t = rng.random()
        color = tuple(clamp(dark[i] * t + light[i] * (1.0 - t) + rng.randint(-8, 8)) for i in range(3))
        d.ellipse((x - r, y - r, x + r, y + r), fill=rgba(color))
        hd.ellipse((x - r, y - r, x + r, y + r), fill=clamp(95 + (1.0 - t) * 45))
    for _ in range(pebble_count):
        cx, cy = rng.randint(8, 120), rng.randint(8, 120)
        rx, ry = rng.randint(3, 10), rng.randint(3, 9)
        shade = rng.randint(58, 94)
        d.ellipse((cx - rx, cy - ry, cx + rx, cy + ry), fill=(shade, shade + 3, shade + 4, 255), outline=(33, 32, 30, 255))
        d.arc((cx - rx + 1, cy - ry + 1, cx + rx - 1, cy + ry - 1), 210, 350, fill=(124, 121, 111, 255), width=1)
        hd.ellipse((cx - rx, cy - ry, cx + rx, cy + ry), fill=rng.randint(138, 188))
    image = grunge(image, seed + 222, alpha, 16, 1.2)
    return image, height


def ground_dust(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    image, height = speckled_ground(seed, (101, 84, 58), (58, 49, 39), (149, 121, 77), 10)
    emission = black_map(alpha)
    normal = height_to_normal(height, alpha, 3.2)
    return material_maps(image, alpha, gray_rgba(210, alpha), gray_rgba(14, alpha), height, normal, emission)


def ground_rocky(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    image, height = speckled_ground(seed, (78, 67, 54), (42, 39, 34), (132, 111, 82), 24)
    emission = black_map(alpha)
    normal = height_to_normal(height, alpha, 3.8)
    return material_maps(image, alpha, gray_rgba(220, alpha), gray_rgba(18, alpha), height, normal, emission)


def ground_dark_gravel(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    image, height = speckled_ground(seed, (47, 45, 41), (25, 25, 24), (85, 82, 75), 28)
    emission = black_map(alpha)
    normal = height_to_normal(height, alpha, 4.0)
    return material_maps(image, alpha, gray_rgba(225, alpha), gray_rgba(12, alpha), height, normal, emission)


def draw_crack_network(image: Image.Image, height: Image.Image, seed: int, dark: tuple[int, int, int]) -> None:
    rng = random.Random(seed)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    for _ in range(8):
        x, y = rng.randint(0, 127), rng.randint(0, 127)
        points = [(x, y)]
        for _ in range(rng.randint(3, 6)):
            x += rng.randint(-22, 22)
            y += rng.randint(-18, 22)
            points.append((x, y))
        d.line(points, fill=rgba(dark), width=3, joint="curve")
        d.line(points, fill=(118, 106, 89, 255), width=1)
        hd.line(points, fill=30, width=4)


def ground_cracked(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    image, height = speckled_ground(seed, (100, 79, 52), (56, 45, 35), (152, 119, 72), 6)
    draw_crack_network(image, height, seed + 5, (34, 28, 24))
    emission = black_map(alpha)
    normal = height_to_normal(height, alpha, 4.2)
    return material_maps(image, alpha, gray_rgba(218, alpha), gray_rgba(12, alpha), height, normal, emission)


def ground_cracked_stone(frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    image, height = speckled_ground(seed, (72, 72, 68), (34, 35, 34), (128, 125, 116), 5)
    draw_crack_network(image, height, seed + 77, (22, 23, 23))
    emission = black_map(alpha)
    normal = height_to_normal(height, alpha, 4.8)
    return material_maps(image, alpha, gray_rgba(190, alpha), gray_rgba(20, alpha), height, normal, emission)


def rock_cluster_base(seed: int, ore: bool, frame_index: int, frame_count: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    ad = ImageDraw.Draw(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    centers = ((39, 78, 27), (68, 62, 32), (91, 83, 25), (58, 96, 20), (89, 43, 18), (32, 101, 9), (109, 101, 10))
    rock_paths: list[list[tuple[int, int]]] = []
    for cx, cy, radius in centers:
        sides = rng.randint(8, 12)
        points = []
        for i in range(sides):
            angle = i / sides * math.tau + rng.uniform(-0.14, 0.14)
            rr = radius * rng.uniform(0.7, 1.1)
            points.append((round(cx + math.cos(angle) * rr), round(cy + math.sin(angle) * rr * 0.78)))
        rock_paths.append(points)
    for index, points in enumerate(rock_paths):
        shade = rng.randint(62, 93)
        ad.polygon(points, fill=255)
        d.polygon(points, fill=(shade, shade + 3, shade + 4, 255), outline=(24, 25, 25, 255))
        hd.polygon(points, fill=132 + index * 10)
        cx = sum(x for x, _ in points) / len(points)
        cy = sum(y for _, y in points) / len(points)
        for vertex in points[::2]:
            highlight = (clamp(shade + 52), clamp(shade + 51), clamp(shade + 47), 255)
            shadow = (clamp(shade - 32), clamp(shade - 30), clamp(shade - 27), 255)
            d.line((vertex, (round(cx), round(cy))), fill=highlight if vertex[1] < cy else shadow, width=1)
    alpha = alpha.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.GaussianBlur(0.35))
    if ore:
        p = pulse(frame_index, frame_count)
        vein_color = (0, 220, 255)
        vein_paths = (
            ((43, 69), (58, 62), (75, 67), (93, 54)),
            ((53, 90), (69, 78), (84, 86), (103, 75)),
            ((36, 82), (50, 86), (66, 76)),
        )
        for path in vein_paths:
            d.line(path, fill=(8, 119, 145, 255), width=3)
            for a, b in zip(path, path[1:]):
                emission = glow_line(emission, (a[0], a[1], b[0], b[1]), vein_color, 4, p, 3.0)
                hd.line((a, b), fill=208, width=3)
    image.putalpha(alpha)
    diffuse = gray_rgba(210, alpha)
    gloss = gray_rgba(28 if not ore else 52, alpha)
    height_rgba = gray_rgba_from_l(height, alpha)
    normal = height_to_normal(height_rgba, alpha, 4.8)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def greenery_common(seed: int, kind: str, frame_index: int, frame_count: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    ad = ImageDraw.Draw(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    greens = ((31, 69, 35), (43, 92, 45), (66, 112, 58), (25, 58, 34), (78, 122, 72))

    def leaf(cx: int, cy: int, length: int, angle: float, width: int, color: tuple[int, int, int]) -> None:
        dx = math.cos(angle) * length
        dy = math.sin(angle) * length
        px = -math.sin(angle) * width
        py = math.cos(angle) * width
        points = [
            (round(cx - px), round(cy - py)),
            (round(cx + dx * 0.55), round(cy + dy * 0.55)),
            (round(cx + px), round(cy + py)),
            (round(cx + dx), round(cy + dy)),
        ]
        ad.polygon(points, fill=210)
        d.polygon(points, fill=rgba(color, 220), outline=rgba(tuple(max(c - 26, 0) for c in color), 150))
        hd.polygon(points, fill=120 + min(length, 28) * 3)

    if kind == "greenery_moss_patch":
        for _ in range(38):
            x, y = rng.randint(8, 120), rng.randint(12, 116)
            rx, ry = rng.randint(5, 18), rng.randint(3, 10)
            color = rng.choice(greens)
            ad.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rng.randint(70, 145))
            d.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rgba(color, rng.randint(90, 165)))
            hd.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rng.randint(28, 72))
    elif kind == "greenery_grass_clumps":
        for _ in range(14):
            base_x, base_y = rng.randint(14, 114), rng.randint(18, 114)
            for _ in range(rng.randint(6, 10)):
                leaf(base_x, base_y, rng.randint(12, 27), rng.uniform(-2.35, -0.75), rng.randint(2, 4), rng.choice(greens))
    elif kind == "greenery_ferns":
        for _ in range(8):
            cx, cy = rng.randint(18, 110), rng.randint(22, 112)
            stem_angle = rng.uniform(-2.7, -0.45)
            end = (round(cx + math.cos(stem_angle) * 31), round(cy + math.sin(stem_angle) * 31))
            d.line((cx, cy, end[0], end[1]), fill=(34, 82, 38, 210), width=2)
            ad.line((cx, cy, end[0], end[1]), fill=190, width=3)
            hd.line((cx, cy, end[0], end[1]), fill=125, width=2)
            for i in range(5):
                t = (i + 1) / 6
                sx = round(cx + (end[0] - cx) * t)
                sy = round(cy + (end[1] - cy) * t)
                leaf(sx, sy, rng.randint(8, 15), stem_angle + rng.choice((-1.0, 1.0)), 2, rng.choice(greens))
    elif kind == "greenery_vines":
        for _ in range(7):
            x, y = rng.randint(0, 32), rng.randint(14, 116)
            points = []
            for step in range(7):
                points.append((x + step * rng.randint(14, 22), y + round(math.sin(step * 1.3 + rng.random()) * rng.randint(4, 12))))
            d.line(points, fill=(31, 80, 38, 210), width=3, joint="curve")
            ad.line(points, fill=185, width=5, joint="curve")
            hd.line(points, fill=120, width=3, joint="curve")
            for px, py in points[1::2]:
                leaf(px, py, rng.randint(8, 15), rng.uniform(-2.6, -0.55), 2, rng.choice(greens))
    elif kind == "greenery_low_bush":
        for _ in range(9):
            cx, cy = rng.randint(20, 108), rng.randint(26, 110)
            for _ in range(10):
                leaf(cx + rng.randint(-8, 8), cy + rng.randint(-6, 6), rng.randint(9, 20), rng.uniform(-math.pi, math.pi), rng.randint(3, 5), rng.choice(greens))
    elif kind == "greenery_glow_lichen":
        p = pulse(frame_index, frame_count)
        for _ in range(32):
            x, y = rng.randint(9, 119), rng.randint(10, 118)
            rx, ry = rng.randint(3, 9), rng.randint(2, 6)
            color = rng.choice(greens)
            ad.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rng.randint(95, 170))
            d.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rgba(color, 160))
            hd.ellipse((x - rx, y - ry, x + rx, y + ry), fill=70)
            if rng.random() < 0.45:
                emission = glow_dot(emission, (x, y), 2, (0, 210, 255), p * rng.uniform(0.35, 0.8))
                d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=(28, 155, 154, 210))
    else:
        raise ValueError(f"Unknown greenery kind: {kind}")

    alpha = alpha.filter(ImageFilter.GaussianBlur(0.35))
    image.putalpha(alpha)
    diffuse = gray_rgba(235, alpha)
    gloss = gray_rgba(8, alpha)
    normal = height_to_normal(height, alpha, 2.8)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


REF_ORANGE = (255, 133, 22)
REF_CYAN = (0, 208, 255)
REF_METAL = (31, 32, 32)
REF_METAL_DARK = (18, 19, 20)
REF_METAL_LIGHT = (82, 84, 82)
REF_RIM = (7, 8, 9)


def reference_metal_base(seed: int, alpha: Image.Image, base: tuple[int, int, int] = REF_METAL) -> Image.Image:
    image = refined_base(alpha, seed, base, 12)
    rng = random.Random(seed + 601)
    d = ImageDraw.Draw(image)
    for _ in range(16):
        x = rng.randint(8, 118)
        y = rng.randint(8, 118)
        length = rng.randint(8, 26)
        angle = rng.choice((0.0, math.pi / 2, rng.uniform(-0.35, 0.35), rng.uniform(2.8, 3.4)))
        x2 = round(x + math.cos(angle) * length)
        y2 = round(y + math.sin(angle) * length)
        shade = rng.randint(52, 92)
        d.line((x, y, x2, y2), fill=(shade, shade, shade, 95), width=1)
    return image


def reference_plate(
    image: Image.Image,
    height: Image.Image,
    box: tuple[int, int, int, int],
    fill: tuple[int, int, int] = (36, 37, 37),
    raised: int = 142,
    radius: int = 2,
    bevel: int = 1,
) -> None:
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.rounded_rectangle(box, radius=radius, fill=rgba(fill), outline=rgba(REF_RIM), width=2)
    if bevel:
        draw_bevel(d, box, (98, 99, 96), (5, 6, 7), bevel)
    hd.rounded_rectangle(box, radius=radius, fill=raised)
    inset = (box[0] + 4, box[1] + 4, box[2] - 4, box[3] - 4)
    if inset[0] < inset[2] and inset[1] < inset[3]:
        d.rounded_rectangle(inset, radius=max(1, radius - 1), outline=(68, 70, 69, 170), width=1)
        hd.rounded_rectangle(inset, radius=max(1, radius - 1), outline=max(0, raised - 34), width=1)


def reference_bolts(
    image: Image.Image,
    height: Image.Image,
    points: tuple[tuple[int, int], ...],
    radius: int = 2,
) -> None:
    for x, y in points:
        draw_bolt(image, height, x, y, radius)


def reference_recess_line(
    image: Image.Image,
    height: Image.Image,
    xy: tuple[int, int, int, int],
    width: int = 3,
) -> None:
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    d.line(xy, fill=(5, 6, 7, 255), width=width)
    d.line(xy, fill=(69, 71, 70, 170), width=1)
    hd.line(xy, fill=42, width=width)


def reference_light_bar(
    image: Image.Image,
    emission: Image.Image,
    box: tuple[int, int, int, int],
    color: tuple[int, int, int],
    strength: float,
) -> Image.Image:
    return draw_light_bar(image, emission, box, color, strength)


def reference_floor_asset(kind: str, frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = refined_alpha(4, 2)
    p = pulse(frame_index, frame_count)
    image = reference_metal_base(seed, alpha, (28, 29, 29))
    height = Image.new("L", (TILE, TILE), 88)
    emission = black_map(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)

    reference_plate(image, height, (6, 6, 121, 121), (34, 35, 35), 134, 2, 1)
    reference_bolts(image, height, ((14, 14), (114, 14), (14, 114), (114, 114)), 2)

    if kind == "floor_panel":
        reference_plate(image, height, (13, 12, 62, 116), (38, 39, 39), 132, 2, 1)
        reference_plate(image, height, (66, 12, 115, 116), (38, 39, 39), 132, 2, 1)
        reference_recess_line(image, height, (64, 12, 64, 116), 5)
        for y in (28, 64, 100):
            reference_recess_line(image, height, (20, y, 56, y), 2)
            reference_recess_line(image, height, (72, y, 108, y), 2)
        emission = reference_light_bar(image, emission, (60, 46, 68, 83), REF_ORANGE, p)
    elif kind == "floor_grate":
        reference_plate(image, height, (15, 15, 113, 113), (37, 38, 38), 128, 2, 1)
        draw_vent(image, height, (25, 35, 103, 93), horizontal=False)
        reference_recess_line(image, height, (64, 18, 64, 110), 5)
        for y in (24, 104):
            reference_recess_line(image, height, (20, y, 108, y), 3)
        for y in (46, 58, 70, 82):
            emission = reference_light_bar(image, emission, (60, y, 68, y + 5), REF_ORANGE, p * 0.85)
    elif kind == "floor_xbrace":
        reference_plate(image, height, (12, 12, 116, 116), (32, 33, 33), 126, 2, 1)
        for xy in ((22, 22, 106, 106), (106, 22, 22, 106)):
            d.line(xy, fill=(7, 8, 9, 255), width=18)
            d.line(xy, fill=(77, 78, 76, 255), width=12)
            d.line(xy, fill=(25, 26, 27, 255), width=6)
            hd.line(xy, fill=186, width=15)
        reference_plate(image, height, (47, 47, 81, 81), (30, 31, 31), 154, 2, 1)
        for box in ((15, 59, 24, 67), (104, 59, 113, 67)):
            emission = reference_light_bar(image, emission, box, REF_ORANGE, p)
    elif kind == "floor_diamond_plate":
        reference_plate(image, height, (10, 10, 118, 118), (32, 33, 33), 136, 2, 1)
        for x in range(-26, 154, 10):
            d.line((x, 24, x + 76, 100), fill=(73, 74, 71, 190), width=1)
            d.line((x + 76, 24, x, 100), fill=(12, 13, 14, 220), width=1)
            hd.line((x, 24, x + 76, 100), fill=162, width=1)
            hd.line((x + 76, 24, x, 100), fill=72, width=1)
    else:
        raise ValueError(f"Unknown reference floor kind: {kind}")

    draw_corner_brackets(image, height, 12)
    draw_scratches(image, seed + 19, 26)
    return panel_maps(image, alpha, height, emission, roughness=112, gloss_level=88, normal_strength=6.2)


def reference_wall_asset(kind: str, frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    p = pulse(frame_index, frame_count)
    if kind == "door_frame":
        outer = refined_alpha(4, 4)
        alpha = subtract_mask(outer, (34, 35, 94, 116), 4)
    else:
        alpha = refined_alpha(4, 2)
    image = reference_metal_base(seed, alpha, (27, 28, 28))
    height = Image.new("L", (TILE, TILE), 70 if kind == "door_frame" else 84)
    emission = black_map(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)

    if kind == "wall_panel":
        reference_plate(image, height, (7, 7, 121, 121), (32, 33, 33), 132, 2, 1)
        for box in ((13, 14, 40, 61), (43, 14, 70, 61), (73, 14, 101, 61), (13, 67, 51, 113), (54, 67, 91, 113), (94, 67, 115, 113)):
            reference_plate(image, height, box, (39, 40, 40), 118, 1, 1)
        draw_vent(image, height, (96, 70, 113, 106), horizontal=True)
        emission = reference_light_bar(image, emission, (104, 26, 111, 57), REF_ORANGE, p)
        for y in (30, 42, 54):
            emission = reference_light_bar(image, emission, (58, y, 64, y + 5), REF_ORANGE, p * 0.65)
        reference_bolts(image, height, ((15, 15), (113, 15), (15, 113), (113, 113), (42, 64), (91, 64)), 2)
    elif kind == "wall_corner":
        reference_plate(image, height, (7, 7, 121, 38), (39, 40, 40), 150, 2, 1)
        reference_plate(image, height, (7, 7, 38, 121), (39, 40, 40), 150, 2, 1)
        reference_plate(image, height, (43, 43, 121, 121), (29, 30, 31), 110, 2, 1)
        for xy in ((20, 22, 96, 22), (22, 20, 22, 96)):
            d.line(xy, fill=(4, 40, 46, 255), width=5)
            emission = glow_line(emission, xy, REF_CYAN, 4, p, 3.0)
        reference_bolts(image, height, ((16, 16), (113, 16), (16, 113), (42, 42), (113, 113)), 2)
    elif kind == "door_frame":
        reference_plate(image, height, (8, 8, 120, 120), (35, 36, 36), 118, 2, 1)
        d.rounded_rectangle((25, 20, 103, 121), radius=6, fill=(10, 11, 12, 255), outline=(89, 91, 88, 255), width=4)
        d.rounded_rectangle((35, 35, 93, 116), radius=4, fill=(0, 0, 0, 0))
        hd.rounded_rectangle((25, 20, 103, 121), radius=6, fill=196)
        hd.rounded_rectangle((35, 35, 93, 116), radius=4, fill=0)
        for box, color in (
            ((42, 15, 86, 21), REF_ORANGE),
            ((26, 52, 32, 94), REF_ORANGE),
            ((96, 52, 102, 94), REF_ORANGE),
        ):
            emission = reference_light_bar(image, emission, box, color, p)
        reference_bolts(image, height, ((18, 18), (110, 18), (18, 112), (110, 112)), 2)
    elif kind == "pipe_corner":
        reference_plate(image, height, (7, 7, 121, 121), (30, 31, 31), 118, 2, 1)
        for offset, width, shade, raised in (
            (0, 15, (9, 10, 11), 80),
            (4, 9, (55, 57, 56), 172),
            (7, 3, (91, 93, 90), 205),
        ):
            d.line((18, 48 + offset, 79, 48 + offset, 79, 96), fill=rgba(shade), width=width, joint="curve")
            hd.line((18, 48 + offset, 79, 48 + offset, 79, 96), fill=raised, width=width)
            d.line((48 + offset, 18, 48 + offset, 79, 96, 79), fill=rgba(shade), width=width, joint="curve")
            hd.line((48 + offset, 18, 48 + offset, 79, 96, 79), fill=raised, width=width)
        emission = reference_light_bar(image, emission, (91, 87, 104, 98), REF_ORANGE, p)
        reference_bolts(image, height, ((19, 48), (48, 19), (79, 96), (96, 79), (112, 16), (16, 112)), 2)
    else:
        raise ValueError(f"Unknown reference wall kind: {kind}")

    draw_scratches(image, seed + 41, 24)
    return panel_maps(image, alpha, height, emission, roughness=105, gloss_level=92, normal_strength=6.0)


def reference_ground_asset(kind: str, frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    alpha = full_alpha()
    if kind == "ground_dust":
        image, height = speckled_ground(seed, (115, 87, 55), (61, 45, 32), (166, 125, 75), 18)
    elif kind == "ground_rocky":
        image, height = speckled_ground(seed, (82, 70, 54), (38, 34, 30), (140, 116, 82), 34)
    elif kind == "ground_dark_gravel":
        image, height = speckled_ground(seed, (41, 40, 37), (18, 18, 18), (83, 80, 72), 40)
    elif kind == "ground_cracked":
        image, height = speckled_ground(seed, (109, 83, 50), (56, 42, 29), (159, 113, 65), 7)
        draw_crack_network(image, height, seed + 31, (31, 24, 20))
    elif kind == "ground_cracked_stone":
        image, height = speckled_ground(seed, (70, 70, 66), (30, 31, 31), (128, 126, 115), 7)
        draw_crack_network(image, height, seed + 63, (18, 19, 19))
    else:
        raise ValueError(f"Unknown reference ground kind: {kind}")

    if kind in ("ground_cracked", "ground_cracked_stone"):
        draw_crack_network(image, height, seed + 101, (20, 18, 16) if kind == "ground_cracked" else (14, 15, 15))
    image = grunge(image, seed + 515, alpha, 10, 1.0)
    emission = black_map(alpha)
    normal = height_to_normal(height, alpha, 4.2 if "cracked" in kind else 3.7)
    gloss_level = 12 if kind != "ground_cracked_stone" else 20
    return material_maps(image, alpha, gray_rgba(218, alpha), gray_rgba(gloss_level, alpha), height, normal, emission)


def reference_rock_asset(seed: int, ore: bool, frame_index: int, frame_count: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    ad = ImageDraw.Draw(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    centers = ((39, 79, 27), (67, 61, 31), (90, 82, 25), (56, 96, 18), (91, 44, 16), (29, 104, 9), (111, 104, 9))

    for index, (cx, cy, radius) in enumerate(centers):
        sides = rng.randint(7, 11)
        points: list[tuple[int, int]] = []
        for i in range(sides):
            angle = i / sides * math.tau + rng.uniform(-0.18, 0.18)
            rr = radius * rng.uniform(0.68, 1.12)
            points.append((round(cx + math.cos(angle) * rr), round(cy + math.sin(angle) * rr * 0.74)))
        shade = rng.randint(55, 86)
        ad.polygon(points, fill=255)
        d.polygon(points, fill=(shade, shade + 3, shade + 3, 255), outline=(15, 16, 16, 255))
        hd.polygon(points, fill=126 + index * 11)
        local_cx = round(sum(x for x, _ in points) / len(points))
        local_cy = round(sum(y for _, y in points) / len(points))
        for vertex in points:
            facet = (clamp(shade + 48), clamp(shade + 47), clamp(shade + 43), 190) if vertex[1] < local_cy else (clamp(shade - 28), clamp(shade - 27), clamp(shade - 25), 210)
            d.line((vertex, (local_cx, local_cy)), fill=facet, width=1)
            hd.line((vertex, (local_cx, local_cy)), fill=166 if vertex[1] < local_cy else 94, width=1)

    alpha = alpha.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.GaussianBlur(0.35))
    if ore:
        p = pulse(frame_index, frame_count)
        vein_color = REF_CYAN
        vein_paths = (
            ((41, 71), (56, 62), (76, 68), (94, 55)),
            ((51, 90), (69, 77), (87, 86), (104, 74)),
            ((37, 82), (50, 86), (67, 75)),
            ((65, 54), (74, 43), (88, 45)),
        )
        for path in vein_paths:
            d.line(path, fill=(7, 126, 151, 255), width=3, joint="curve")
            for a, b in zip(path, path[1:]):
                emission = glow_line(emission, (a[0], a[1], b[0], b[1]), vein_color, 4, p, 3.2)
                hd.line((a, b), fill=214, width=3)
        for x, y in ((101, 94), (32, 101), (88, 36)):
            emission = glow_dot(emission, (x, y), 2, REF_ORANGE, p * 0.55)

    image.putalpha(alpha)
    diffuse = gray_rgba(210, alpha)
    gloss = gray_rgba(24 if not ore else 54, alpha)
    normal = height_to_normal(height, alpha, 4.9)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


def reference_greenery_asset(seed: int, kind: str, frame_index: int, frame_count: int) -> dict[str, Image.Image]:
    rng = random.Random(seed)
    alpha = Image.new("L", (TILE, TILE), 0)
    image = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    height = Image.new("L", (TILE, TILE), 0)
    emission = black_map(alpha)
    ad = ImageDraw.Draw(alpha)
    d = ImageDraw.Draw(image)
    hd = ImageDraw.Draw(height)
    greens = ((34, 76, 35), (48, 94, 42), (76, 119, 55), (24, 58, 31), (97, 128, 61))

    def blade(cx: int, cy: int, length: int, angle: float, width: int, color: tuple[int, int, int], alpha_value: int = 230) -> None:
        dx = math.cos(angle) * length
        dy = math.sin(angle) * length
        px = -math.sin(angle) * width
        py = math.cos(angle) * width
        points = [
            (round(cx - px), round(cy - py)),
            (round(cx + dx * 0.45), round(cy + dy * 0.45)),
            (round(cx + px), round(cy + py)),
            (round(cx + dx), round(cy + dy)),
        ]
        ad.polygon(points, fill=alpha_value)
        d.polygon(points, fill=rgba(color, alpha_value), outline=rgba(tuple(max(c - 30, 0) for c in color), 120))
        d.line((cx, cy, round(cx + dx), round(cy + dy)), fill=rgba(tuple(clamp(c + 24) for c in color), 105), width=1)
        hd.polygon(points, fill=110 + min(length, 32) * 3)

    def leaf_cluster(cx: int, cy: int, count: int, spread: int, min_len: int, max_len: int) -> None:
        for _ in range(count):
            ox = rng.randint(-spread, spread)
            oy = rng.randint(-spread, spread)
            angle = math.atan2(oy if oy else rng.choice((-1, 1)), ox if ox else rng.choice((-1, 1)))
            blade(cx + ox // 3, cy + oy // 3, rng.randint(min_len, max_len), angle, rng.randint(3, 6), rng.choice(greens))

    if kind == "greenery_moss_patch":
        for _ in range(48):
            x, y = rng.randint(8, 120), rng.randint(12, 116)
            rx, ry = rng.randint(5, 18), rng.randint(3, 10)
            color = rng.choice(greens)
            ad.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rng.randint(70, 155))
            d.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rgba(color, rng.randint(95, 170)))
            hd.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rng.randint(28, 72))
    elif kind == "greenery_grass_clumps":
        for _ in range(10):
            bx, by = rng.randint(15, 113), rng.randint(36, 116)
            for _ in range(rng.randint(10, 16)):
                blade(bx, by, rng.randint(16, 35), rng.uniform(-2.7, -0.45), rng.randint(2, 4), rng.choice(greens))
    elif kind == "greenery_ferns":
        for _ in range(7):
            cx, cy = rng.randint(20, 108), rng.randint(28, 112)
            angle = rng.uniform(-2.75, -0.38)
            end = (round(cx + math.cos(angle) * 35), round(cy + math.sin(angle) * 35))
            d.line((cx, cy, end[0], end[1]), fill=(35, 83, 35, 220), width=2)
            ad.line((cx, cy, end[0], end[1]), fill=200, width=3)
            hd.line((cx, cy, end[0], end[1]), fill=135, width=2)
            for i in range(6):
                t = (i + 1) / 7
                sx = round(cx + (end[0] - cx) * t)
                sy = round(cy + (end[1] - cy) * t)
                blade(sx, sy, rng.randint(8, 16), angle + rng.choice((-1.05, 1.05)), 2, rng.choice(greens))
    elif kind == "greenery_vines":
        for _ in range(9):
            x, y = rng.randint(0, 34), rng.randint(14, 116)
            points = []
            for step in range(7):
                points.append((x + step * rng.randint(14, 21), y + round(math.sin(step * 1.35 + rng.random()) * rng.randint(5, 13))))
            d.line(points, fill=(32, 82, 35, 220), width=3, joint="curve")
            ad.line(points, fill=195, width=5, joint="curve")
            hd.line(points, fill=120, width=3, joint="curve")
            for px, py in points[1::2]:
                blade(px, py, rng.randint(8, 16), rng.uniform(-2.8, -0.35), 2, rng.choice(greens))
    elif kind == "greenery_low_bush":
        leaf_cluster(64, 72, 72, 43, 9, 22)
    elif kind == "greenery_glow_lichen":
        p = pulse(frame_index, frame_count)
        for _ in range(42):
            x, y = rng.randint(8, 120), rng.randint(9, 119)
            rx, ry = rng.randint(3, 9), rng.randint(2, 6)
            color = rng.choice(greens)
            ad.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rng.randint(105, 175))
            d.ellipse((x - rx, y - ry, x + rx, y + ry), fill=rgba(color, 165))
            hd.ellipse((x - rx, y - ry, x + rx, y + ry), fill=72)
            if rng.random() < 0.5:
                emission = glow_dot(emission, (x, y), 2, REF_CYAN, p * rng.uniform(0.35, 0.85))
                d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=(25, 148, 150, 215))
    else:
        raise ValueError(f"Unknown reference greenery kind: {kind}")

    alpha = alpha.filter(ImageFilter.GaussianBlur(0.3))
    image.putalpha(alpha)
    diffuse = gray_rgba(235, alpha)
    gloss = gray_rgba(8, alpha)
    normal = height_to_normal(height, alpha, 3.0)
    return material_maps(image, alpha, diffuse, gloss, height, normal, emission)


REFERENCE_SHEET_PATH = OUT_ROOT / "reference" / "new_texture_reference.png"
REFERENCE_GRID_X_BOUNDS = (
    (18, 186),
    (204, 362),
    (380, 540),
    (557, 716),
    (733, 889),
    (908, 1067),
    (1085, 1237),
)
REFERENCE_GRID_Y_BOUNDS = (
    (18, 186),
    (201, 377),
    (396, 566),
    (584, 752),
    (767, 935),
    (953, 1092),
    (1107, 1233),
)
REFERENCE_SHEET_CELLS = {
    "floor_panel": (0, 0),
    "floor_grate": (0, 1),
    "floor_xbrace": (0, 2),
    "floor_diamond_plate": (0, 4),
    "wall_panel": (1, 3),
    "wall_corner": (0, 6),
    "door_frame": (1, 5),
    "pipe_corner": (2, 6),
    "ground_dust": (3, 0),
    "ground_rocky": (3, 1),
    "ground_cracked": (3, 2),
    "ground_dark_gravel": (3, 4),
    "ground_cracked_stone": (3, 5),
    "rock_cluster": (4, 2),
    "ore_rocks": (4, 5),
    "greenery_moss_patch": (6, 0),
    "greenery_grass_clumps": (5, 1),
    "greenery_ferns": (5, 2),
    "greenery_vines": (6, 6),
    "greenery_low_bush": (5, 5),
    "greenery_glow_lichen": (6, 0),
}
REFERENCE_CUTOUT_KINDS = {
    "door_frame",
    "rock_cluster",
    "ore_rocks",
    "greenery_moss_patch",
    "greenery_grass_clumps",
    "greenery_ferns",
    "greenery_vines",
    "greenery_low_bush",
    "greenery_glow_lichen",
}
REFERENCE_FULL_ALPHA_KINDS = set(REFERENCE_SHEET_CELLS) - REFERENCE_CUTOUT_KINDS


def load_reference_sheet() -> Image.Image:
    if not REFERENCE_SHEET_PATH.exists():
        raise FileNotFoundError(f"Missing reference sheet: {REFERENCE_SHEET_PATH}")
    return Image.open(REFERENCE_SHEET_PATH).convert("RGBA")


def crop_reference_cell(kind: str) -> Image.Image:
    if kind not in REFERENCE_SHEET_CELLS:
        raise ValueError(f"No reference cell mapped for {kind}")
    row, column = REFERENCE_SHEET_CELLS[kind]
    x0, x1 = REFERENCE_GRID_X_BOUNDS[column]
    y0, y1 = REFERENCE_GRID_Y_BOUNDS[row]
    sheet = load_reference_sheet()
    crop = sheet.crop((x0, y0, x1, y1))
    return normalize_reference_crop(crop, kind)


def visible_bbox(image: Image.Image, threshold: int = 8, padding: int = 2) -> tuple[int, int, int, int]:
    rgb = np.array(image.convert("RGB"))
    mask = rgb.max(axis=2) > threshold
    if not mask.any():
        return (0, 0, image.width, image.height)
    ys, xs = np.where(mask)
    x0 = max(0, int(xs.min()) - padding)
    y0 = max(0, int(ys.min()) - padding)
    x1 = min(image.width, int(xs.max()) + padding + 1)
    y1 = min(image.height, int(ys.max()) + padding + 1)
    return (x0, y0, x1, y1)


def mask_bbox(mask: np.ndarray, padding: int, width: int, height: int) -> tuple[int, int, int, int]:
    if not mask.any():
        return (0, 0, width, height)
    ys, xs = np.where(mask)
    x0 = max(0, int(xs.min()) - padding)
    y0 = max(0, int(ys.min()) - padding)
    x1 = min(width, int(xs.max()) + padding + 1)
    y1 = min(height, int(ys.max()) + padding + 1)
    return (x0, y0, x1, y1)


def largest_component(mask: np.ndarray) -> np.ndarray:
    h, w = mask.shape
    seen = np.zeros((h, w), dtype=bool)
    best: list[tuple[int, int]] = []
    for y in range(h):
        for x in range(w):
            if not mask[y, x] or seen[y, x]:
                continue
            stack = [(x, y)]
            seen[y, x] = True
            component: list[tuple[int, int]] = []
            while stack:
                cx, cy = stack.pop()
                component.append((cx, cy))
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < w and 0 <= ny < h and mask[ny, nx] and not seen[ny, nx]:
                        seen[ny, nx] = True
                        stack.append((nx, ny))
            if len(component) > len(best):
                best = component
    cleaned = np.zeros((h, w), dtype=bool)
    for x, y in best:
        cleaned[y, x] = True
    return cleaned


def reference_cutout_threshold(kind: str) -> int:
    if kind in ("rock_cluster", "ore_rocks"):
        return 38
    return 18


def border_connected_background(max_channel: np.ndarray, threshold: int) -> np.ndarray:
    h, w = max_channel.shape
    background = max_channel <= threshold
    visited = np.zeros((h, w), dtype=bool)
    stack: list[tuple[int, int]] = []
    for x in range(w):
        if background[0, x]:
            stack.append((x, 0))
        if background[h - 1, x]:
            stack.append((x, h - 1))
    for y in range(h):
        if background[y, 0]:
            stack.append((0, y))
        if background[y, w - 1]:
            stack.append((w - 1, y))
    while stack:
        x, y = stack.pop()
        if visited[y, x] or not background[y, x]:
            continue
        visited[y, x] = True
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < w and 0 <= ny < h and not visited[ny, nx] and background[ny, nx]:
                stack.append((nx, ny))
    return visited


def normalize_reference_crop(image: Image.Image, kind: str) -> Image.Image:
    if kind.startswith("greenery_"):
        rgb = np.array(image.convert("RGB"))
        mask = largest_component(rgb.max(axis=2) > reference_cutout_threshold(kind))
        bbox = mask_bbox(mask, 5, image.width, image.height)
    else:
        bbox = visible_bbox(
            image,
            threshold=reference_cutout_threshold(kind) if kind in REFERENCE_CUTOUT_KINDS else 8,
            padding=5 if kind in REFERENCE_CUTOUT_KINDS else 2,
        )
    trimmed = image.crop(bbox)
    if kind not in REFERENCE_CUTOUT_KINDS:
        return trimmed.resize((TILE, TILE), Image.Resampling.LANCZOS)

    available = TILE - 10
    scale = min(available / max(1, trimmed.width), available / max(1, trimmed.height))
    size = (max(1, round(trimmed.width * scale)), max(1, round(trimmed.height * scale)))
    resized = trimmed.resize(size, Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    canvas.alpha_composite(resized, ((TILE - size[0]) // 2, (TILE - size[1]) // 2))
    return canvas


def alpha_from_reference(image: Image.Image, kind: str) -> Image.Image:
    if kind in REFERENCE_FULL_ALPHA_KINDS:
        return full_alpha()
    rgb = np.array(image.convert("RGB"))
    if kind in ("rock_cluster", "ore_rocks"):
        max_channel = rgb.max(axis=2)
        background = border_connected_background(max_channel, 34)
        mask = (~background) & (max_channel > 8)
        alpha = Image.fromarray(np.where(mask, 255, 0).astype(np.uint8), "L")
        alpha = alpha.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.MinFilter(3)).filter(ImageFilter.GaussianBlur(0.45))
    else:
        mask = rgb.max(axis=2) > reference_cutout_threshold(kind)
        if kind.startswith("greenery_"):
            mask = largest_component(mask)
        alpha_arr = np.where(mask, 255, 0).astype(np.uint8)
        alpha = Image.fromarray(alpha_arr, "L")
        alpha = alpha.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.MinFilter(3)).filter(ImageFilter.GaussianBlur(0.35))
    return alpha


def reference_emission_from_image(
    image: Image.Image,
    alpha: Image.Image,
    frame_index: int,
    frame_count: int,
) -> Image.Image:
    p = pulse(frame_index, frame_count)
    rgb = np.array(image.convert("RGB")).astype(np.float32)
    r = rgb[:, :, 0]
    g = rgb[:, :, 1]
    b = rgb[:, :, 2]
    orange = (r > 130) & (g > 45) & (r > g * 1.25) & (b < 115)
    cyan = (b > 120) & (g > 95) & (r < 100)
    mask = orange | cyan
    out = np.zeros((TILE, TILE, 4), dtype=np.uint8)
    boost = 1.25 * p
    out[:, :, :3] = np.clip(rgb * boost, 0, 255).astype(np.uint8)
    out[:, :, 3] = np.where(mask, 255, 0).astype(np.uint8)
    crisp = Image.fromarray(out, "RGBA")
    halo = crisp.filter(ImageFilter.GaussianBlur(4))
    emission = add_rgb(halo, crisp)
    emission.putalpha(alpha)
    return emission


def reference_height_from_image(image: Image.Image, alpha: Image.Image, kind: str) -> Image.Image:
    rgb = np.array(image.convert("RGB")).astype(np.float32)
    luma = rgb[:, :, 0] * 0.299 + rgb[:, :, 1] * 0.587 + rgb[:, :, 2] * 0.114
    visible = np.array(alpha) > 0
    if visible.any():
        low = float(np.percentile(luma[visible], 8))
        high = float(np.percentile(luma[visible], 94))
    else:
        low, high = 0.0, 255.0
    if high <= low:
        high = low + 1.0
    normalized = np.clip((luma - low) / (high - low), 0.0, 1.0)
    if kind.startswith("ground_"):
        base, span = 58, 150
    elif kind.startswith("greenery_"):
        base, span = 26, 120
    elif kind in ("rock_cluster", "ore_rocks"):
        base, span = 38, 175
    else:
        base, span = 46, 166
    height_arr = np.clip(base + normalized * span, 0, 255).astype(np.uint8)
    height_arr[~visible] = 0
    return Image.fromarray(height_arr, "L").filter(ImageFilter.GaussianBlur(0.25))


def repair_reference_cutout_color(image: Image.Image, alpha: Image.Image, kind: str) -> Image.Image:
    if kind not in ("rock_cluster", "ore_rocks"):
        return image
    arr = np.array(image.convert("RGBA"))
    alpha_arr = np.array(alpha)
    max_channel = arr[:, :, :3].max(axis=2)
    strong = (alpha_arr > 10) & (max_channel > reference_cutout_threshold(kind))
    if strong.any():
        fill = np.percentile(arr[:, :, :3][strong], 28, axis=0)
        fill = np.clip(fill * 0.72, 26, 92).astype(np.uint8)
    else:
        fill = np.array((44, 43, 40), dtype=np.uint8)
    weak = (alpha_arr > 10) & (max_channel <= reference_cutout_threshold(kind))
    arr[:, :, :3][weak] = fill
    arr[:, :, 3] = alpha_arr
    return Image.fromarray(arr, "RGBA")


def reference_sheet_asset(kind: str, frame_index: int, frame_count: int, seed: int) -> dict[str, Image.Image]:
    image = crop_reference_cell(kind)
    alpha = alpha_from_reference(image, kind)
    image = repair_reference_cutout_color(image, alpha, kind)
    image_rgba = image.convert("RGBA")
    image_rgba.putalpha(alpha)

    if kind == "greenery_glow_lichen":
        rng = random.Random(seed + frame_index * 13)
        visible_points = np.argwhere(np.array(alpha) > 80)
        glow_draw = ImageDraw.Draw(image_rgba)
        if len(visible_points) > 0:
            for _ in range(18):
                y, x = visible_points[rng.randrange(len(visible_points))]
                radius = rng.choice((1, 1, 2))
                glow_draw.ellipse(
                    (x - radius, y - radius, x + radius, y + radius),
                    fill=(24, 166, 158, 220),
                )

    emission = reference_emission_from_image(image_rgba, alpha, frame_index, frame_count)
    height = reference_height_from_image(image_rgba, alpha, kind)
    normal_strength = 4.4
    roughness = 118
    gloss_level = 80
    if kind.startswith("ground_"):
        normal_strength = 3.7
        roughness = 218
        gloss_level = 14 if kind != "ground_cracked_stone" else 22
    elif kind.startswith("greenery_"):
        normal_strength = 2.8
        roughness = 235
        gloss_level = 8
    elif kind in ("rock_cluster", "ore_rocks"):
        normal_strength = 4.8
        roughness = 210
        gloss_level = 26 if kind == "rock_cluster" else 54
    elif kind in ("floor_diamond_plate", "floor_grate", "floor_xbrace", "floor_panel"):
        normal_strength = 5.8
        roughness = 112
        gloss_level = 92
    normal = height_to_normal(height, alpha, normal_strength)
    return material_maps(
        image_rgba,
        alpha,
        gray_rgba(roughness, alpha),
        gray_rgba(gloss_level, alpha),
        height,
        normal,
        emission,
    )


def save_image(image: Image.Image, path: Path) -> None:
    tmp_path = path.with_name(f".{path.stem}.tmp{path.suffix}")
    image.save(tmp_path)
    tmp_path.replace(path)


def build_maps(spec: AssetSpec, frame_index: int, angle: int = ANGLE) -> dict[str, Image.Image]:
    seed = spec.seed + frame_index * 97 + angle * 31
    if spec.kind in REFERENCE_SHEET_CELLS:
        return reference_sheet_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "floor_panel":
        return reference_floor_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "floor_grate":
        return reference_floor_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "floor_xbrace":
        return reference_floor_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "floor_diamond_plate":
        return reference_floor_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "wall_panel":
        return reference_wall_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "wall_corner":
        return reference_wall_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "ortho_wall_panel":
        return ortho_wall_panel(frame_index, spec.frames, seed, angle)
    if spec.kind == "ortho_wall_corner":
        return ortho_wall_corner(frame_index, spec.frames, seed, angle)
    if spec.kind == "door_frame":
        return reference_wall_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "pipe_corner":
        return reference_wall_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "ground_dust":
        return reference_ground_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "ground_cracked":
        return reference_ground_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "ground_rocky":
        return reference_ground_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "ground_dark_gravel":
        return reference_ground_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "ground_cracked_stone":
        return reference_ground_asset(spec.kind, frame_index, spec.frames, seed)
    if spec.kind == "rock_cluster":
        return reference_rock_asset(seed, ore=False, frame_index=frame_index, frame_count=spec.frames)
    if spec.kind == "ore_rocks":
        return reference_rock_asset(seed, ore=True, frame_index=frame_index, frame_count=spec.frames)
    if spec.kind.startswith("greenery_"):
        return reference_greenery_asset(seed, spec.kind, frame_index, spec.frames)
    raise ValueError(f"Unknown asset kind: {spec.kind}")


def write_atlas(spec: AssetSpec) -> None:
    folder = OUT_ROOT / spec.folder
    folder.mkdir(parents=True, exist_ok=True)
    frame_numbers = FRAME_NUMBERS[spec.frames]
    atlas_width = TILE * len(frame_numbers) * len(spec.angles)
    atlas_height = TILE * len(MAP_ORDER)
    atlas = Image.new("RGBA", (atlas_width, atlas_height), (0, 0, 0, 0))

    sprites: list[str] = []
    for angle_column, angle in enumerate(spec.angles):
        for frame_column, frame_number in enumerate(frame_numbers):
            column = angle_column * len(frame_numbers) + frame_column
            maps = build_maps(spec, frame_column, angle)
            for row, prefix in enumerate(MAP_ORDER):
                x = column * TILE
                y = row * TILE
                atlas.alpha_composite(maps[prefix], (x, y))
                sprites.append(
                    f'    <sprite n="{prefix}{frame_number:04d}_{angle}" x="{x}" y="{y}" w="{TILE}" h="{TILE}"/>'
                )

    png_path = folder / f"{spec.name}.png"
    xml_path = folder / f"{spec.name}.xml"
    save_image(atlas, png_path)
    xml_path.write_text(
        "\n".join(
            (
                '<?xml version="1.0" encoding="UTF-8"?>',
                "<!-- Generated by tools/generate_scifi_base_assets.py -->",
                f'<TextureAtlas imagePath="{escape(png_path.name)}" width="{atlas_width}" height="{atlas_height}">',
                *sprites,
                "</TextureAtlas>",
                "",
            )
        ),
        encoding="utf-8",
    )


def write_preview() -> None:
    columns = 4
    rows = math.ceil(len(ASSETS) / columns)
    width = columns * TILE
    height = rows * TILE
    transparent_preview = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    preview = Image.new("RGBA", (width, height), (18, 20, 22, 255))
    for index, spec in enumerate(ASSETS):
        maps = build_maps(spec, 0)
        x = (index % columns) * TILE
        y = (index // columns) * TILE
        transparent_preview.alpha_composite(maps["Image"], (x, y))
        preview.alpha_composite(maps["Image"], (x, y))
    save_image(preview, OUT_ROOT / "scifi_base_preview.png")
    save_image(transparent_preview, OUT_ROOT / "scifi_base_preview_transparent.png")

    checker = Image.new("RGBA", (width, height), (34, 36, 38, 255))
    checker_draw = ImageDraw.Draw(checker)
    for y in range(0, height, 16):
        for x in range(0, width, 16):
            if (x // 16 + y // 16) % 2 == 0:
                checker_draw.rectangle((x, y, x + 15, y + 15), fill=(62, 64, 66, 255))
    checker.alpha_composite(transparent_preview)
    save_image(checker, OUT_ROOT / "scifi_base_preview_checker.png")

    ground_cycle = [
        spec for spec in ASSETS
        if spec.kind in ("ground_dust", "ground_rocky", "ground_dark_gravel", "ground_cracked_stone")
    ]
    greenery_specs = [spec for spec in ASSETS if spec.kind.startswith("greenery_")]
    overlay = Image.new("RGBA", (3 * TILE, math.ceil(len(greenery_specs) / 3) * TILE), (0, 0, 0, 0))
    for index, greenery in enumerate(greenery_specs):
        ground = ground_cycle[index % len(ground_cycle)]
        tile = build_maps(ground, 0)["Image"].copy()
        tile.alpha_composite(build_maps(greenery, 0)["Image"])
        x = (index % 3) * TILE
        y = (index // 3) * TILE
        overlay.alpha_composite(tile, (x, y))
    save_image(overlay, OUT_ROOT / "scifi_base_greenery_overlay_preview.png")

    ortho_specs = [spec for spec in ASSETS if spec.kind.startswith("ortho_wall_")]
    if ortho_specs:
        angle_preview = Image.new(
            "RGBA",
            (len(ORTHO_WALL_ANGLES) * TILE, len(ortho_specs) * TILE),
            (18, 20, 22, 255),
        )
        for row, spec in enumerate(ortho_specs):
            for column, angle in enumerate(ORTHO_WALL_ANGLES):
                tile = build_maps(spec, 0, angle)["Image"]
                angle_preview.alpha_composite(tile, (column * TILE, row * TILE))
        save_image(angle_preview, OUT_ROOT / "scifi_base_ortho_wall_angles_preview.png")


def write_readme() -> None:
    lines = [
        "# Sci-Fi Base Texture Starter Pack",
        "",
        "Generated semi-realistic sci-fi base-building and environment assets.",
        "Each asset folder contains a packed PNG atlas and TexturePacker-style XML.",
        "The current replacement set uses `reference/new_texture_reference.png` as",
        "the source style sheet for the top-down walls, floors, ground, rocks and",
        "foliage, then derives the material rows used by the renderer.",
        "",
        "All atlases use 128x128 unrotated frames at angle 0. Animated assets use",
        "frame numbers 0001, 0003, 0005 and 0007 to match the existing stepped",
        "animation convention.",
        "",
        "Orthographic wall atlases are drawn for a 60-degree camera pitch on the",
        "X axis and include yaw states at 0, 45, 90, 135, 180, 225, 270 and 315",
        "degrees for camera-rotation swaps.",
        "",
        "Material rows included in every atlas:",
        "",
        "- Alpha",
        "- Diffuse",
        "- Emission",
        "- Gloss",
        "- Height",
        "- Image",
        "- Normal",
        "",
        "Load an animated asset with:",
        "",
        "```cpp",
        "animator->loadTexturePackerAtlas(",
        '    "res/textures/scifi_base/base_floor_panel/Base_Floor_Panel.xml",',
        '    "Base_Floor_Panel",',
        "    6.0f,",
        "    true",
        ");",
        'animator->setDefaultState("Base_Floor_Panel_0");',
        "```",
        "",
        "Assets:",
        "",
    ]
    for spec in ASSETS:
        if len(spec.angles) == 1:
            state_note = f"State: `{spec.name}_{spec.angles[0]}`"
        else:
            states = ", ".join(f"`{spec.name}_{angle}`" for angle in spec.angles)
            state_note = f"States: {states}"
        lines.append(f"- `{spec.folder}/{spec.name}.xml`: {spec.note} {state_note}, suggested FPS: `{spec.fps}`.")
    lines.extend(
        (
            "",
        "The `scifi_base_preview.png` file is a quick contact sheet of the first",
        "albedo frame from each asset on a dark background. The transparent and",
        "checkerboard previews are useful for inspecting alpha edges. The greenery",
        "overlay preview shows the foliage layers composited onto ground tiles.",
        "`scifi_base_ortho_wall_angles_preview.png` shows the orthographic wall",
        "angle states in order from 0 through 315 degrees.",
        "",
    )
    )
    (OUT_ROOT / "README.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    for spec in ASSETS:
        write_atlas(spec)
    write_preview()
    write_readme()
    print(f"Wrote {len(ASSETS)} sci-fi texture assets to {OUT_ROOT}")


if __name__ == "__main__":
    main()
