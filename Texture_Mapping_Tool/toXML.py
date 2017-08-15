import os
fout = open('result.xml', 'w')
filenames = [f for f in os.listdir() if f.startswith('viewMatrix')]
for index, filename in enumerate(filenames):
    with open(filename) as f:
        content = f.readlines()
    warpedImageFilename = content[0].strip()
    originalImageFilename = content[1].strip()
    viewMatrix = ' '.join(['%.16e'%float(num) for num in content[2].split()])
    output='''          <camera id="{index}" label="{filename}" sensor_id="0" enabled="true">
            <transform>{transform}</transform>
            <orientation>6</orientation>
          </camera>
    '''.format(index=index, filename=warpedImageFilename, transform=viewMatrix)
    print(output)
    fout.write(output)
fout.close()

