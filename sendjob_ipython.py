from optparse import OptionParser
import platform
import os
from IPython.parallel import Client


def get_osname():
    osname = platform.system()
    if osname == 'Darwin':
        osname = 'OSX'
    return osname


def get_rendertemplate(renderer):
    if renderer == 'blender':
        filename = 'blender_sg.py'
    if renderer == 'maya':
        filename = 'maya_sg.py'
    if renderer == 'mentalray':
        filename = 'mentalray_sg.py'
    return filename    


def run_script_with_env(script, env_dict):
    DRQUEUE_OS = env_dict['DRQUEUE_OS']
    DRQUEUE_ETC = env_dict['DRQUEUE_ETC']
    DRQUEUE_FRAME = env_dict['DRQUEUE_FRAME']
    DRQUEUE_BLOCKSIZE = env_dict['DRQUEUE_BLOCKSIZE']
    DRQUEUE_ENDFRAME = env_dict['DRQUEUE_ENDFRAME']
    SCENE = env_dict['SCENE']
    RENDER_TYPE = env_dict['RENDER_TYPE']
    return execfile(script)


def main():
    # parse arguments
    parser = OptionParser()
    parser.usage = "%prog [options] -r renderer -f scenefile"
    parser.add_option("-s", "--startframe", dest="startframe", default=1,
                      help="first frame")
    parser.add_option("-e", "--endframe", dest="endframe", default=1,
                      help="last frame")
    parser.add_option("-b", "--blocksize", dest="blocksize", default=1,
                      help="size of block")
    parser.add_option("-f", "--scenefile", dest="scenefile", default=1,
                      help="path to scenefile")
    parser.add_option("-r", "--renderer", dest="renderer",
                      help="render type (maya|blender|mentalray)")
    parser.add_option("-w", "--wait", dest="wait", default=False,
                      help="wait for job to finish")
    parser.add_option("-v", "--verbose",
                      action="store_false", dest="verbose", default=True,
                      help="verbose output")
    (options, args) = parser.parse_args()

    # initialize IPython
    client = Client()
    dview = client[:]
    lbview = client.load_balanced_view()

    tasks = list()
    task_frames = range(int(options.startframe), int(options.endframe)+1, int(options.blocksize))

    for x in task_frames:
        # prepare script input
        env_dict = {
        'DRQUEUE_OS' : get_osname(),
        'DRQUEUE_ETC' : os.getenv('DRQUEUE_ROOT') + "/etc",
        'DRQUEUE_FRAME' : x,
        'DRQUEUE_BLOCKSIZE' : int(options.blocksize),
        'DRQUEUE_ENDFRAME' : int(options.endframe),
        'SCENE' : options.scenefile,
        'RENDER_TYPE' : "animation"
        }

        # run task on cluster
        render_script = os.getenv('DRQUEUE_ROOT') + "/etc/" + get_rendertemplate(options.renderer)
        ar = lbview.apply(run_script_with_env, render_script, env_dict)
        tasks.append(ar)

    if options.wait:
        for x in tasks:
            x.wait()
            cpl = x.metadata.completed
            print("Task %s finished with status '%s' on engine %i at %i-%02i-%02i %02i:%02i:%02i." % (x.metadata.msg_id, x.status, x.metadata.engine_id, cpl.year, cpl.month, cpl.day, cpl.hour, cpl.minute, cpl.second))
            #print(x.stderr)
            #print(x.stdout)


if __name__ == "__main__":
    main()

