#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import json
import logging
import pathlib
import urllib.parse
import urllib.request
import hashlib
from datetime import datetime
from o3de import manifest, utils, validation

logger = logging.getLogger('o3de.repo')
logging.basicConfig(format=utils.LOG_FORMAT)

REPO_IMPLICIT_SCHEMA_VERSION = "0.0.0"

REPO_SCHEMA_VERSION_1_0_0 = "1.0.0"

def get_cache_file_uri(uri: str):
    parsed_uri = urllib.parse.urlparse(uri)
    uri_sha256 = hashlib.sha256(parsed_uri.geturl().encode())
    cache_file = manifest.get_o3de_cache_folder() / str(uri_sha256.hexdigest() + '.json')
    return cache_file, parsed_uri

def sanitized_repo_uri(repo_uri: str) -> str or None:
    # remove excess whitespace and any trailing slashes
    return repo_uri.strip().rstrip('/') if repo_uri else None

def get_repo_manifest_uri(repo_uri: str) -> str or None:
    if not repo_uri:
        logger.error(f'Repo URI cannot be empty.')
        return None

    return f'{repo_uri}/repo.json'

def download_repo_manifest(manifest_uri: str) -> pathlib.Path or None:
    cache_file, parsed_uri = get_cache_file_uri(manifest_uri)

    git_provider = utils.get_git_provider(parsed_uri)
    if git_provider:
        parsed_uri = git_provider.get_specific_file_uri(parsed_uri)

    result = utils.download_file(parsed_uri, cache_file, True)

    return cache_file if result == 0 else None

def download_object_manifests(repo_data):
    cache_folder = manifest.get_o3de_cache_folder()
    # A repo may not contain all types of object.
    manifest_download_list = []
    try:
        manifest_download_list.append((repo_data['engines'], 'engine.json'))
    except KeyError:
        pass
    try:
        manifest_download_list.append((repo_data['projects'], 'project.json'))
    except KeyError:
        pass
    try:
        manifest_download_list.append((repo_data['gems'], 'gem.json'))
    except KeyError:
        pass
    try:
        manifest_download_list.append((repo_data['templates'], 'template.json'))
    except KeyError:
        pass
    try:
        manifest_download_list.append((repo_data['restricted'], 'restricted.json'))
    except KeyError:
        pass

    for o3de_object_uris, manifest_json in manifest_download_list:
        for o3de_object_uri in o3de_object_uris:
            manifest_json_uri = f'{o3de_object_uri}/{manifest_json}'
            cache_file, parsed_uri = get_cache_file_uri(manifest_json_uri)

            git_provider = utils.get_git_provider(parsed_uri)
            if git_provider:
                parsed_uri = git_provider.get_specific_file_uri(parsed_uri)

            download_file_result = utils.download_file(parsed_uri, cache_file, True)
            if download_file_result != 0:
                return download_file_result
    return 0

def get_repo_schema_version(repo_data: dict):
    return repo_data.get("$schemaVersion", REPO_IMPLICIT_SCHEMA_VERSION)

def validate_remote_repo(repo_uri: str, validate_contained_objects: bool = False) -> bool:
    manifest_uri = get_repo_manifest_uri(repo_uri)

    if not manifest_uri:
        return False

    cache_file = download_repo_manifest(manifest_uri)

    if not cache_file:
        logger.error(f'Could not download file at {manifest_uri}')
        return False

    if not validation.valid_o3de_repo_json(cache_file):
        logger.error(f'Repository JSON {cache_file} could not be loaded or is missing required values')
        return False

    if validate_contained_objects:
        repo_data = {}
        with cache_file.open('r') as f:
            try:
                repo_data = json.load(f)
            except json.JSONDecodeError as e:
                logger.error(f'Invalid JSON - {cache_file} could not be loaded')
                return False
            
        repo_schema_version = get_repo_schema_version(repo_data)

        if repo_schema_version == REPO_IMPLICIT_SCHEMA_VERSION:
            if download_object_manifests(repo_data) != 0:
                return False
            gem_set = get_gem_json_paths_from_cached_repo(repo_uri)
            for gem_json in gem_set:
                if not validation.valid_o3de_gem_json(gem_json):
                    logger.error(f'Invalid gem JSON - {gem_json} could not be loaded or is missing required values')
                    return False
            project_set = get_project_json_paths_from_cached_repo(repo_uri)
            for project_json in project_set:
                if not validation.valid_o3de_project_json(project_json):
                    logger.error(f'Invalid project JSON - {project_json} could not be loaded or is missing required values')
                    return False
            template_set = get_template_json_paths_from_cached_repo(repo_uri)
            for template_json in template_set:
                if not validation.valid_o3de_template_json(template_json):
                    logger.error(f'Invalid template JSON - {template_json} could not be loaded or is missing required values')
                    return False
                
        elif repo_schema_version == REPO_SCHEMA_VERSION_1_0_0:
            gem_list = repo_data.get("gems", [])
            for gem_json in gem_list:
                if not validation.valid_o3de_gem_json_data(gem_json):
                    logger.error(f'Invalid gem JSON - {gem_json} is missing required values')
                    return False

                # validate versioning info
                if "versions_data" in gem_json:
                    versions_data = gem_json["versions_data"]
                    for version in versions_data:
                        if not all(key in version for key in ['last_updated', 'version']):
                            logger.error("Invalid gem JSON - {gem_json} Both last_updated and version fields must be defined for each entry in the versions_data field")
                            return False
                        
                        source_control_uri_defined = "source_control_uri" in version
                        
                        download_source_uri_defined = "download_source_uri" in version
                        origin_uri_defined = "origin_uri" in version
                        
                        # prevent mixing fields intended for backwards compatibility (using XOR to verify)
                        download_origin_defined = ((download_source_uri_defined and not origin_uri_defined) or
                                                   (not download_source_uri_defined and origin_uri_defined))

                        if not (source_control_uri_defined or download_origin_defined):
                            logger.error(f"Invalid gem JSON - {gem_json} At least one of source_control_uri or download_source_uri must be defined")
                            return False

            project_list = repo_data.get("projects", [])
            for project_json in project_list:
                if not validation.valid_o3de_project_json_data(project_json):
                    logger.error(f'Invalid project JSON - {project_json} is missing required values')
                    return False                    

            template_list = repo_data.get("templates", [])
            for template_json in template_list:
                if not validation.valid_o3de_template_json_data(template_json):
                    logger.error(f'Invalid template JSON - {template_json} is missing required values')
                    return False

    return True

def process_add_o3de_repo(file_name: str or pathlib.Path,
                          repo_set: set) -> int:
    file_name = pathlib.Path(file_name).resolve()
    if not validation.valid_o3de_repo_json(file_name):
        logger.error(f'Repository JSON {file_name} could not be loaded or is missing required values')
        return 1
    cache_folder = manifest.get_o3de_cache_folder()

    repo_data = {}
    with file_name.open('r') as f:
        try:
            repo_data = json.load(f)
        except json.JSONDecodeError as e:
            logger.error(f'{file_name} failed to load: {str(e)}')
            return 1

    with file_name.open('w') as f:
        try:
            time_now = datetime.now()
            time_str = time_now.strftime('%d/%m/%Y %H:%M')
            repo_data.update({'last_updated': time_str})
            f.write(json.dumps(repo_data, indent=4) + '\n')
        except Exception as e:
            logger.error(f'{file_name} failed to save: {str(e)}')
            return 1

    if download_object_manifests(repo_data) != 0:
        return 1

    # Having a repo is also optional
    repo_list = []
    try:
        repo_list.extend(repo_data['repos'])
    except KeyError:
        pass

    for repo in repo_list:
        if repo not in repo_set:
            repo_set.add(repo)
            repo_uri = f'{repo}/repo.json'
            cache_file, parsed_uri = get_cache_file_uri(repo_uri)
            
            download_file_result = utils.download_file(parsed_uri, cache_file, True)
            if download_file_result != 0:
                return download_file_result

            return process_add_o3de_repo(cache_file, repo_set)
    return 0


def get_object_json_paths_from_cached_repo(repo_uri: str, repo_key: str, object_manifest_filename: str) -> set:
    url = f'{repo_uri}/repo.json'
    cache_file, _ = get_cache_file_uri(url)

    o3de_object_set = set()

    file_name = pathlib.Path(cache_file).resolve()
    if not file_name.is_file():
        logger.error(f'Could not find cached repository json file for {repo_uri}. Try refreshing the repository.')
        return o3de_object_set

    with file_name.open('r') as f:
        try:
            repo_data = json.load(f)
        except json.JSONDecodeError as e:
            logger.error(f'{file_name} failed to load: {str(e)}')
            return o3de_object_set

        # Get list of objects, then add all json paths to the list if they exist in the cache
        repo_objects = []
        try:
            repo_objects.append((repo_data[repo_key], object_manifest_filename + '.json'))
        except KeyError:
            pass

        for o3de_object_uris, manifest_json in repo_objects:
            for o3de_object_uri in o3de_object_uris:
                manifest_json_uri = f'{o3de_object_uri}/{manifest_json}'
                cache_object_json_filepath, _ = get_cache_file_uri(manifest_json_uri)
                
                if cache_object_json_filepath.is_file():
                    o3de_object_set.add(cache_object_json_filepath)
                else:
                    logger.warning(f'Could not find cached {repo_key} json file {cache_object_json_filepath} for {o3de_object_uri} in repo {repo_uri}')

    return o3de_object_set

def get_gem_json_paths_from_cached_repo(repo_uri: str) -> set:
    return get_object_json_paths_from_cached_repo(repo_uri, 'gems', 'gem')

def get_gem_json_paths_from_all_cached_repos() -> set:
    json_data = manifest.load_o3de_manifest()
    gem_set = set()

    for repo_uri in json_data.get('repos', []):
        gem_set.update(get_gem_json_paths_from_cached_repo(repo_uri))

    return gem_set

def get_project_json_paths_from_cached_repo(repo_uri: str) -> set:
    return get_object_json_paths_from_cached_repo(repo_uri, 'projects', 'project')

def get_project_json_paths_from_all_cached_repos() -> set:
    json_data = manifest.load_o3de_manifest()
    project_set = set()

    for repo_uri in json_data.get('repos', []):
        project_set.update(get_project_json_paths_from_cached_repo(repo_uri))

    return project_set

def get_template_json_paths_from_cached_repo(repo_uri: str) -> set:
    return get_object_json_paths_from_cached_repo(repo_uri, 'templates', 'template')

def get_template_json_paths_from_all_cached_repos() -> set:
    json_data = manifest.load_o3de_manifest()
    template_set = set()

    for repo_uri in json_data.get('repos', []):
        template_set.update(get_template_json_paths_from_cached_repo(repo_uri))

    return template_set

def refresh_repo(repo_uri: str,
                 repo_set: set = None) -> int:
    if not repo_set:
        repo_set = set()

    repo_uri = f'{repo_uri}/repo.json'
    cache_file = download_repo_manifest(repo_uri)
    if not cache_file:
        logger.error(f'Repo json {repo_uri} could not download.')
        return 1

    if not validation.valid_o3de_repo_json(cache_file):
        logger.error(f'Repo json {repo_uri} is not valid.')
        cache_file.unlink()
        return 1

    return process_add_o3de_repo(cache_file, repo_set)

def refresh_repos() -> int:
    json_data = manifest.load_o3de_manifest()
    result = 0

    # set will stop circular references
    repo_set = set()

    for repo_uri in json_data.get('repos', []):
        if repo_uri not in repo_set:
            repo_set.add(repo_uri)

            last_failure = refresh_repo(repo_uri, repo_set)
            if last_failure:
                result = last_failure

    return result


def search_repo(manifest_json_data: dict,
                engine_name: str = None,
                project_name: str = None,
                gem_name: str = None,
                template_name: str = None,
                restricted_name: str = None) -> dict or None:
    
    repo_schema_version = get_repo_schema_version(manifest_json_data)

    if repo_schema_version == REPO_IMPLICIT_SCHEMA_VERSION:        
        if isinstance(engine_name, str):
            o3de_object = search_o3de_manifest_for_object(manifest_json_data, 'engines', 'engine.json', 'engine_name', engine_name)
        elif isinstance(project_name, str):
            o3de_object = search_o3de_manifest_for_object(manifest_json_data, 'projects', 'project.json', 'project_name', project_name)
        elif isinstance(gem_name, str):
            o3de_object = search_o3de_manifest_for_object(manifest_json_data, 'gems', 'gem.json', 'gem_name', gem_name)
        elif isinstance(template_name, str):
            o3de_object = search_o3de_manifest_for_object(manifest_json_data, 'templates', 'template.json', 'template_name', template_name)
        elif isinstance(restricted_name, str):
            o3de_object = search_o3de_manifest_for_object(manifest_json_data, 'restricted', 'restricted.json', 'restricted_name', restricted_name)
        else:
            return None
        
    elif repo_schema_version == REPO_SCHEMA_VERSION_1_0_0:
        #search for the o3de object from inside repos object 
        if isinstance(engine_name, str):
            o3de_object = search_o3de_repo_for_object(manifest_json_data, 'engines', 'engine_name', engine_name)
        elif isinstance(project_name, str):
            o3de_object = search_o3de_repo_for_object(manifest_json_data, 'projects', 'project_name', project_name)
        elif isinstance(gem_name, str):
            o3de_object = search_o3de_repo_for_object(manifest_json_data, 'gems', 'gem_name', gem_name)
        elif isinstance(template_name, str):
            o3de_object = search_o3de_repo_for_object(manifest_json_data, 'templates', 'template_name', template_name)
        elif isinstance(restricted_name, str):
            o3de_object = search_o3de_repo_for_object(manifest_json_data, 'restricted', 'restricted_name', restricted_name)
        else:
            return None
        
    if o3de_object:
        o3de_object['repo_name'] = manifest_json_data['repo_name']
        return o3de_object
    # recurse into the repos object to search for the o3de object
    o3de_object_uris = []
    try:
        o3de_object_uris = manifest_json_data['repos']
    except KeyError:
        pass

    manifest_json = 'repo.json'
    search_func = lambda manifest_json_data: search_repo(manifest_json_data, engine_name, project_name, gem_name, template_name)
    return search_o3de_object(manifest_json, o3de_object_uris, search_func)


def search_o3de_repo_for_object(repo_json_data: dict, manifest_attribute:str, target_json_key:str, target_name: str):
    o3de_objects = repo_json_data.get(manifest_attribute, [])
    for o3de_object in o3de_objects:
        if o3de_object.get(target_json_key, '') == target_name:
            return o3de_object
    return None

def search_o3de_manifest_for_object(manifest_json_data: dict, manifest_attribute: str, target_manifest_json: str, target_json_key: str, target_name: str):
    o3de_object_uris = manifest_json_data.get(manifest_attribute, [])
    search_func = lambda manifest_json_data: manifest_json_data if manifest_json_data.get(target_json_key, '') == target_name else None
    return search_o3de_object(target_manifest_json, o3de_object_uris, search_func)


def search_o3de_object(manifest_json, o3de_object_uris, search_func):
    # Search for the o3de object based on the supplied object name in the current repo
    for o3de_object_uri in o3de_object_uris:
        manifest_uri = f'{o3de_object_uri}/{manifest_json}'
        cache_file, _ = get_cache_file_uri(manifest_uri)

        if cache_file.is_file():
            with cache_file.open('r') as f:
                try:
                    manifest_json_data = json.load(f)
                except json.JSONDecodeError as e:
                    logger.warning(f'{cache_file} failed to load: {str(e)}')
                else:
                    result_json_data = search_func(manifest_json_data)
                    if result_json_data:
                        return result_json_data
    return None
