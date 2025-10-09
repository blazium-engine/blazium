using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using Microsoft.Build.Construction;
using Microsoft.Build.Evaluation;
using Microsoft.Build.Locator;
using NuGet.Frameworks;

namespace GodotTools.ProjectEditor
{
    public sealed class MSBuildProject
    {
        internal ProjectRootElement Root { get; set; }

        public bool HasUnsavedChanges { get; set; }

        public void Save() => Root.Save();

        public MSBuildProject(ProjectRootElement root)
        {
            Root = root;
        }
    }

    public static partial class ProjectUtils
    {

        private static readonly string[] _platformNames =
        {
            "windows",
            "linuxbsd",
            "macos",
            "android",
            "ios",
            "web",
        };

        public static void MSBuildLocatorRegisterLatest(out Version version, out string path)
        {
            var instance = MSBuildLocator.QueryVisualStudioInstances()
                .OrderByDescending(x => x.Version)
                .First();
            MSBuildLocator.RegisterInstance(instance);
            version = instance.Version;
            path = instance.MSBuildPath;
        }

        public static void MSBuildLocatorRegisterMSBuildPath(string msbuildPath)
            => MSBuildLocator.RegisterMSBuildPath(msbuildPath);

        public static MSBuildProject? Open(string path)
        {
            var root = ProjectRootElement.Open(path, ProjectCollection.GlobalProjectCollection, preserveFormatting: true);
            return root != null ? new MSBuildProject(root) : null;
        }

        public static void UpgradeProjectIfNeeded(MSBuildProject project, string projectName)
        {
            // NOTE: The order in which changes are made to the project is important.

            // Migrate to MSBuild project Sdks style if using the old style.
            MigrateToProjectSdksStyle(project, projectName);

            EnsureGodotSdkIsUpToDate(project);
        }

        public static void MigrateToProjectSdksStyle(MSBuildProject project, string projectName)
        {
            var origRoot = project.Root;

            if (!string.IsNullOrEmpty(origRoot.Sdk))
                return;

            project.Root = ProjectGenerator.GenGameProject(projectName);
            project.Root.FullPath = origRoot.FullPath;
            project.HasUnsavedChanges = true;
        }

        public static void EnsureGodotSdkIsUpToDate(MSBuildProject project)
        {
            var root = project.Root;
            string godotSdkAttrValue = ProjectGenerator.GodotSdkAttrValue;

            if (!string.IsNullOrEmpty(root.Sdk) &&
                root.Sdk.Trim().Equals(godotSdkAttrValue, StringComparison.OrdinalIgnoreCase))
                return;

            root.Sdk = godotSdkAttrValue;
            project.HasUnsavedChanges = true;
        }

    }
}
