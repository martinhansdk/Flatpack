# Flatpack - Publication TODO List

Checklist for publishing Flatpack on Autodesk App Store.

**Last Updated:** February 27, 2026

---

## 🎨 Visual Assets

### High Priority

- [ ] **Create 512x512 icon**
  - Current icon is 120x120
  - Can upscale from existing icon.svg
  - Required format: PNG with transparency
  - Tools: Inkscape, Illustrator, or online SVG to PNG converter
  - Save as: `doc/icon-512.png`

- [ ] **Take screenshots** (minimum 5 required)
  - Screenshot 1: Main export dialog with face selection visible
  - Screenshot 2: Multiple faces selected in viewport (highlighted)
  - Screenshot 3: File selection dialog showing .dxf/.svg options
  - Screenshot 4: Example output file opened in laser cutter software
  - Screenshot 5: Before/after - complex design → exported layout
  - Screenshot 6 (bonus): Finished laser-cut project made with Flatpack
  - Recommended resolution: 1920x1080 or higher
  - Format: PNG or JPG
  - Save to: `doc/screenshots/`

### Medium Priority

- [ ] **Create promotional video** (optional but recommended)
  - Length: 30-60 seconds
  - Follow script in `doc/MARKETPLACE_LISTING.md`
  - Show: problem → solution → features → result
  - Upload to YouTube and embed link
  - Estimated time: 2-3 hours

---

## 📧 Contact Information

### Required Updates

- [ ] **Set up support email**
  - Create dedicated email: support@yourproject.com or use existing
  - Consider using GitHub email if comfortable
  - Alternative: Use GitHub Issues exclusively

- [ ] **Update all documentation with email**
  - [ ] README.md (line with "Add your support email here")
  - [ ] doc/USER_GUIDE.md (in "Getting Help" section)
  - [ ] doc/privacy_policy.md (in "Contact" section)
  - [ ] doc/MARKETPLACE_LISTING.md (in "Support" section)
  - Search for: `[Add your support email` and `[Your support email`

---

## 🔐 Legal & Technical

### Code Signing (Windows Only)

- [ ] **Obtain code signing certificate**
  - Cost: ~$200-500 per year
  - Providers: DigiCert, Sectigo, GlobalSign
  - Required for: Windows .dll signing
  - Not required for: Initial testing or free distribution
  - Priority: Medium (can publish without it, but recommended)

- [ ] **Sign the Windows DLL**
  - Tool: signtool.exe (included with Visual Studio)
  - Add signing step to GitHub Actions workflow
  - Document in build instructions

### Terms of Service

- [ ] **Create Terms of Service** (if required by app store)
  - Can use MIT license as basis
  - Clarify: warranty disclaimer, liability limits
  - Save as: `doc/terms_of_service.md`
  - Review Autodesk's requirements for TOS
  - Priority: Check app store requirements first

### Testing

- [ ] **Test on Windows 10**
  - Install from ZIP
  - Test all features
  - Export DXF and SVG
  - Verify files open in CAM software

- [ ] **Test on Windows 11**
  - Same tests as Windows 10

- [ ] **Test on macOS**
  - Install from ZIP
  - Test all features
  - Export DXF and SVG
  - Verify files open in CAM software

- [ ] **Test with different Fusion 360 versions**
  - Current version
  - Previous version (if accessible)
  - Document minimum supported version

- [ ] **Test edge cases**
  - Very large files (100+ faces)
  - Complex curves
  - Very small tolerance values (0.001mm)
  - Very large tolerance values (1mm)
  - Invalid file paths
  - No write permissions

---

## 🌐 Autodesk Setup

### Account Creation

- [ ] **Register for Autodesk Developer Network**
  - URL: https://www.autodesk.com/developer-network
  - Create account or sign in
  - Complete profile information
  - Verify email

- [ ] **Access Autodesk App Store Developer Portal**
  - Navigate to app submission area
  - Review submission guidelines
  - Note any specific requirements

- [ ] **Complete publisher profile**
  - Company/Individual name
  - Bio/Description
  - Website (GitHub repo URL is fine)
  - Social media links (optional)
  - Support contact information
  - Profile picture/logo

### App Store Submission

- [ ] **Create new app listing**
  - App name: Flatpack
  - Category: Utilities / CAD Tools
  - Short description (copy from MARKETPLACE_LISTING.md)
  - Long description (copy from MARKETPLACE_LISTING.md)

- [ ] **Upload visual assets**
  - 512x512 icon
  - All screenshots
  - Video link (if created)

- [ ] **Set pricing**
  - Free (recommended for initial release)
  - Or set price if monetizing

- [ ] **Upload add-in package**
  - Windows build ZIP
  - macOS build ZIP
  - Or combined ZIP if app store allows

- [ ] **Add documentation links**
  - User guide
  - Privacy policy
  - License information
  - GitHub repository

- [ ] **Select keywords/tags**
  - Use list from MARKETPLACE_LISTING.md
  - "laser cutting", "DXF export", "SVG export", etc.

- [ ] **Complete compliance information**
  - Export compliance
  - Privacy compliance
  - Security review
  - Any required certifications

---

## 📝 Documentation Review

### Final Checks

- [ ] **Review all documentation for accuracy**
  - [ ] README.md
  - [ ] USER_GUIDE.md
  - [ ] privacy_policy.md
  - [ ] MARKETPLACE_LISTING.md

- [ ] **Check all links work**
  - GitHub repository links
  - Issue tracker links
  - External resources

- [ ] **Update version numbers** (if needed)
  - Currently: 1.0.2
  - Update in: Flatpack.manifest, CMakeLists.txt
  - Update in documentation if bumping version

- [ ] **Proofread for typos**
  - Run spell checker
  - Check grammar
  - Verify technical accuracy

---

## 🚀 Pre-Launch

### Code Quality

- [ ] **Run final build on CI**
  - Verify Windows build succeeds
  - Verify macOS build succeeds
  - Download and test artifacts

- [ ] **Review open GitHub issues**
  - Address critical bugs
  - Document known issues
  - Close or comment on stale issues

- [ ] **Update changelog**
  - Add all features since last release
  - Note any breaking changes
  - List bug fixes

### Marketing Preparation

- [ ] **Write launch announcement**
  - Draft blog post or social media posts
  - Highlight key features
  - Include screenshots
  - Add download/install links

- [ ] **Prepare support resources**
  - Create FAQ based on common questions
  - Set up issue templates on GitHub
  - Prepare canned responses for common problems

- [ ] **Notify beta testers** (if any)
  - Thank early users
  - Ask for testimonials
  - Request App Store reviews after launch

---

## 📤 Submission

### Submit to App Store

- [ ] **Final review of submission**
  - All fields completed
  - All assets uploaded
  - Documentation linked
  - Contact info correct

- [ ] **Submit for review**
  - Click submit button
  - Note submission date
  - Save confirmation email/number

- [ ] **Wait for review** (1-4 weeks typical)
  - Monitor email for questions
  - Respond promptly to reviewer feedback
  - Make requested changes quickly

### Alternative Distribution

- [ ] **Create GitHub Release**
  - Tag version: v1.0.2
  - Upload Windows ZIP
  - Upload macOS ZIP
  - Copy release notes
  - Mark as latest release

- [ ] **Announce on GitHub**
  - Update README with release link
  - Pin announcement issue
  - Post in discussions (if enabled)

---

## 📊 Post-Launch

### Monitoring

- [ ] **Monitor user feedback**
  - Check App Store reviews
  - Watch GitHub issues
  - Read support emails

- [ ] **Track metrics**
  - Download count
  - App Store rating
  - GitHub stars/watchers
  - Support request volume

### Maintenance

- [ ] **Plan first update**
  - Address early bug reports
  - Add requested features
  - Improve documentation based on questions

- [ ] **Set up update schedule**
  - Quarterly maintenance updates
  - Security patches as needed
  - Feature releases as planned

---

## ⏱️ Time Estimates

| Task Category | Estimated Time |
|--------------|----------------|
| Visual Assets | 2-4 hours |
| Contact Setup | 1 hour |
| Testing | 4-6 hours |
| Autodesk Account Setup | 2-3 hours |
| App Store Submission | 2-3 hours |
| Documentation Review | 2 hours |
| **Total** | **13-19 hours** |

Plus: App Store review time (1-4 weeks)

---

## 🎯 Priority Levels

### Must Have (Before Submission)
- 512x512 icon
- 5+ screenshots
- Support email configured
- Testing on both platforms
- Autodesk account created

### Should Have (For Good Launch)
- Code signing (Windows)
- Demo video
- Multiple testers
- Comprehensive testing

### Nice to Have (Can Add Later)
- Professional marketing materials
- Video tutorials
- Community building
- Advanced features

---

## 📞 Resources

### Helpful Links
- Autodesk App Store: https://apps.autodesk.com/
- Developer Portal: https://www.autodesk.com/developer-network
- Fusion 360 API Docs: https://help.autodesk.com/view/fusion360/ENU/?guid=GUID-A92A4B10-3781-4925-94C6-47DA85A4F65A
- Code Signing Info: https://docs.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools

### Tools
- Icon creation: Inkscape (free), Adobe Illustrator
- Screenshots: Windows Snipping Tool, macOS Screenshot utility
- Video editing: DaVinci Resolve (free), Adobe Premiere
- Screen recording: OBS Studio (free), Camtasia

---

## ✅ Completion Tracking

**Progress:** 0/35 tasks complete

Update this count as you complete tasks!

**Last Review:** February 27, 2026
**Target Launch Date:** [Set your target date]

---

## 📝 Notes

Use this space for notes, questions, or blockers:

- 
- 
- 

---

**Tip:** Start with visual assets and testing - these take the longest but can be done in parallel with other tasks!
