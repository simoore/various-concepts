"""
Let's have a look at rasterio.

Rasterio is for reading GIS raster data. It aims to present the functionality of GDAL in a more python way.
"""

import matplotlib.pyplot as plt
import rasterio
import rasterio.features
import rasterio.warp

with rasterio.open('RGB.byte.tif') as dataset:

    # Dataset objects have bands. This example has 3 bands.
    print(dataset.count)

    # There are a number of other attributes.
    print(dataset.name)
    print(dataset.mode)
    print(dataset.closed)
    print(dataset.height)
    print(dataset.width)

    # This prints the data type of each band.
    print({i: dtype for i, dtype in zip(dataset.indexes, dataset.dtypes)})

    # The bounds of the iamge mapped to coordinates on the earth
    print(dataset.bounds)

    # The coordinate system of these bounds are:
    print(dataset.crs)

    # Also the affine transformation from pixel space to the above coordinate system is:
    print(dataset.transform)

    # Bands are 1 indexed (following GDALs convention). These are returned as a ndarray.
    print(dataset.read(1))
    print(type(dataset.read(1)))

    # You can index points in there georeferenced space.
    band1 = dataset.read(1)
    x, y = (dataset.bounds.left + 100000, dataset.bounds.top - 50000)
    row, col = dataset.index(x, y)
    print(band1[row, col])

    # You can get the pixel coordinates from spatial coordinates as well.
    print(dataset.xy(dataset.height // 2, dataset.width // 2))

    # Read the dataset's valid data mask as a ndarray.
    mask = dataset.dataset_mask()
    plt.imshow(mask)

    # Extract feature shapes and values from the array.
    for geom, val in rasterio.features.shapes(mask, transform=dataset.transform):

        # Transform shapes from the dataset's own coordinate
        # reference system to CRS84 (EPSG:4326).
        geom = rasterio.warp.transform_geom(dataset.crs, 'EPSG:4326', geom, precision=6)

        # Print GeoJSON shapes to stdout.
        print(geom)

plt.show()
# skimage, pytest and shapely.