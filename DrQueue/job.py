# -*- coding: utf-8 -*-

"""
DrQueue Job submodule
Copyright (C) 2011,2012 Andreas Schroeder

This file is part of DrQueue.

Licensed under GNU General Public License version 3. See LICENSE for details.
"""

import os, datetime
import getpass
import DrQueue


class Job(dict):
    """Subclass of dict for collecting Job attribute values."""
    def __init__(self, name, startframe, endframe, blocksize, renderer, scenefile, retries=1, owner=getpass.getuser(), options={}, created_with=None, limits={}):
        dict.__init__(self)
        # mandatory elements
        jb = {'name' : name,
              'startframe' : int(startframe),
              'endframe' : int(endframe),
              'blocksize' : int(blocksize),
              'renderer' : renderer,
              'scenefile' : scenefile,
              'retries' : int(retries),
              'owner' : owner,
              'submit_time' : datetime.datetime.now(),
              'requeue_time' : False,
              'created_with' : created_with,
              'enabled' : True,
              'limits' : {}
             }
        if name == "":
            raise ValueError("No name of job given!")
            return False
        if not (endframe >= startframe >= 1):
            raise ValueError("Startframe and endframe need to be at least 1!")
            return False
        if blocksize < 1:
            raise ValueError("Blocksize needs to be at least 1!")
            return False
        if DrQueue.check_renderer_support(renderer) == False:
            raise ValueError("Render called \"%s\" not supported!" % renderer)
            return False
        if scenefile == "":
            raise ValueError("No scenefile given!")
            return False
        # optional elements
        if 'renderdir' in options:
            jb['renderdir'] = options['renderdir']
        if 'projectdir' in options:
            jb['projectdir'] = options['projectdir']
        if 'configdir' in options:
            jb['configdir'] = options['configdir']
        if 'imagefile' in options:
            jb['imagefile'] = options['imagefile']
        if 'precommand' in options:
            jb['precommand'] = options['precommand']
        if 'postcommand' in options:
            jb['postcommand'] = options['postcommand']
        if 'viewcommand' in options:
            jb['viewcommand'] = options['viewcommand']
        if 'worldfile' in options:
            jb['worldfile'] = options['worldfile']
        if 'terrainfile' in options:
            jb['terrainfile'] = options['terrainfile']
        if 'composition' in options:
            jb['composition'] = options['composition']
        if 'camera' in options:
            jb['camera'] = options['camera']
        if 'resx' in options:
            jb['resx'] = options['resx']
        if 'resy' in options:
            jb['resy'] = options['resy']
        if 'renderpass' in options:
            jb['renderpass'] = options['renderpass']
        if 'rendertype' in options:
            jb['rendertype'] = options['rendertype']
        if 'fileextension' in options:
            jb['fileextension'] = options['fileextension']
        if 'send_email' in options:
            jb['send_email'] = options['send_email']
        if 'email_recipients' in options:
            jb['email_recipients'] = options['email_recipients']
        if 'custom_command' in options:
            jb['custom_command'] = options_var['custom_command']
        # limits
        if 'os' in limits:
            jb['limits']['os'] = limits['os']
        if 'depend' in limits:
            jb['limits']['depend'] = limits['depend']
        if 'minram' in limits:
            jb['limits']['minram'] = limits['minram']
        if 'mincores' in limits:
            jb['limits']['mincores'] = limits['mincores']
        if 'pool_name' in limits:
            jb['limits']['pool_name'] = limits['pool_name']

        self.update(jb)


    @staticmethod
    def store_db(job):
        import pymongo
        """store job information in MongoDB"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        job_id = jobs.insert(job)
        job['_id'] = str(job['_id'])
        return job_id


    @staticmethod
    def update_db(job):
        import pymongo
        """update job information in MongoDB"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        job_id = jobs.save(job)
        job['_id'] = str(job['_id'])
        return job_id


    @staticmethod
    def query_db(job_id):
        import pymongo
        import bson
        """query job information from MongoDB"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        try:
            job = jobs.find_one({"_id": bson.ObjectId(job_id)})
        except bson.errors.InvalidId:
            print("Format error: Invalid BSON.")
            job = None
        return job


    @staticmethod
    def delete_from_db(job_id):
        import pymongo
        import bson
        """query job information from MongoDB"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        return jobs.remove({"_id": bson.ObjectId(job_id)})


    @staticmethod
    def query_jobnames():
        import pymongo
        """query job names from MongoDB"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        names = []
        for job in jobs.find():
            names.append(job['name'])
        return names


    @staticmethod
    def query_job_by_name(job_name):
        import pymongo
        """query job information from MongoDB by name"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        job = jobs.find_one({"name": job_name})
        return job


    @staticmethod
    def query_job_list():
        import pymongo
        """query list of jobs from MongoDB"""
        connection = pymongo.Connection(os.getenv('DRQUEUE_MONGODB'))
        db = connection['ipythondb']
        jobs = db['drqueue_jobs']
        return list(jobs.find())
        
