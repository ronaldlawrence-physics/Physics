#!/usr/bin/env python3
import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib.patches import Patch

plate_w = 90.0
plate_d = 120.0
glass_h = 2.0
vis_emul_h = 0.4
n_plates = 10
total_h = n_plates * (glass_h + vis_emul_h)

def cuboid_faces(c):
    i = [0,1,2,3, 4,5,6,7, 0,1,5,4, 2,3,7,6, 0,3,7,4, 1,2,6,5]
    return [c[i[j:j+4]] for j in range(0, 24, 4)]

fig = plt.figure(figsize=(14, 10))
ax = fig.add_subplot(111, projection='3d')

gc = '#6a74c1'
ec = '#6ac1c1'

for i in range(n_plates):
    z = i * (glass_h + vis_emul_h)
    for h, c in [(glass_h, gc), (vis_emul_h, ec)]:
        o = np.array([[0,0,z],[plate_w,0,z],[plate_w,plate_d,z],[0,plate_d,z],
                      [0,0,z+h],[plate_w,0,z+h],[plate_w,plate_d,z+h],[0,plate_d,z+h]])
        alpha = 0.5 if c == gc else 0.8
        for f in cuboid_faces(o):
            ax.add_collection3d(Poly3DCollection([f], alpha=alpha,
                facecolor=c, edgecolor='black', linewidth=0.3))
    ax.text(plate_w+1, plate_d/2, z + glass_h + vis_emul_h/2,
            f'Pl{i+1}', fontsize=8, va='center')

ax.text(plate_w/2, -8, -1, '90 mm', fontsize=9, ha='center', va='top')
ax.text(-8, plate_d/2, -1, '120 mm', fontsize=9, ha='center', va='center', rotation=90)

ax.plot([plate_w/2, plate_w/2], [-5, -5], [total_h+8, total_h+0.5],
        color='red', lw=2)
ax.scatter([plate_w/2], [-5], [total_h+0.5], s=150, c='red', marker='v')
ax.text(plate_w/2, -15, total_h+4, 'Beam\ndirection',
        fontsize=10, color='red', ha='center', va='center')

ax.legend(handles=[
    Patch(facecolor=gc, alpha=0.5, label='Glass (2 mm)'),
    Patch(facecolor=ec, alpha=0.8, label='Emulsion (60 um)'),
], loc='upper right', fontsize=10)

ax.set_xlim(-10, plate_w+15)
ax.set_ylim(-25, plate_d+10)
ax.set_zlim(-2, total_h+12)
ax.view_init(elev=22, azim=-55)
ax.set_box_aspect([1, 1.2, 0.35])
ax.set_xlabel('X (mm)')
ax.set_ylabel('Y (mm)')
ax.set_zlabel('Depth (mm)')

plt.tight_layout()
plt.savefig('/home/ronald/JINR/LHEP/Data/Xe_280Mev_2026/analysis/emulsion_stack_3d.png', dpi=200, bbox_inches='tight')
print('Saved')
