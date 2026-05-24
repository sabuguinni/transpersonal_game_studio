#include "Eng_CompilationFixer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"

UEng_CompilationFixer::UEng_CompilationFixer()
{
    bFixInProgress = false;
    InitializeCriticalSystems();
}

void UEng_CompilationFixer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Fixer - Initialized"));
    
    // Run initial analysis
    LastReport = AnalyzeCompilationStatus();
    LogCompilationStatus();
}

void UEng_CompilationFixer::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Fixer - Deinitializing"));
    Super::Deinitialize();
}

void UEng_CompilationFixer::InitializeCriticalSystems()
{
    CriticalSystems.Empty();
    CriticalSystems.Add(TEXT("Eng_CriticalSystemsManager"));
    CriticalSystems.Add(TEXT("Eng_ArchitectureManager"));
    CriticalSystems.Add(TEXT("Eng_CompilationValidator"));
    CriticalSystems.Add(TEXT("Eng_TechnicalArchitecture"));
    CriticalSystems.Add(TEXT("Eng_SystemManager"));
    CriticalSystems.Add(TEXT("BiomeManager"));
    CriticalSystems.Add(TEXT("DinosaurBase"));
    CriticalSystems.Add(TEXT("StudioDirector"));
}

FEng_CompilationReport UEng_CompilationFixer::AnalyzeCompilationStatus()
{
    FEng_CompilationReport Report;
    Report.Status = EEng_CompilationStatus::Compiling;
    
    // Get project source directory
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"), TEXT("Core"));
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    // Find all .h files
    TArray<FString> HeaderFiles;
    PlatformFile.FindFilesRecursively(HeaderFiles, *SourceDir, TEXT(".h"));
    
    // Find all .cpp files
    TArray<FString> CppFiles;
    PlatformFile.FindFilesRecursively(CppFiles, *SourceDir, TEXT(".cpp"));
    
    Report.TotalHeaders = HeaderFiles.Num();
    Report.TotalCppFiles = CppFiles.Num();
    
    // Check for orphaned headers
    for (const FString& HeaderPath : HeaderFiles)
    {
        FString HeaderName = FPaths::GetBaseFilename(HeaderPath);
        FString CppPath = HeaderPath.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!PlatformFile.FileExists(*CppPath))
        {
            FEng_OrphanedHeader OrphanedHeader;
            OrphanedHeader.HeaderName = HeaderName;
            OrphanedHeader.FilePath = HeaderPath;
            OrphanedHeader.bHasCppFile = false;
            
            // Count lines in header
            FString HeaderContent;
            if (FFileHelper::LoadFileToString(HeaderContent, *HeaderPath))
            {
                TArray<FString> Lines;
                HeaderContent.ParseIntoArrayLines(Lines);
                OrphanedHeader.LineCount = Lines.Num();
            }
            
            Report.OrphanedHeaders.Add(OrphanedHeader);
        }
    }
    
    Report.OrphanedCount = Report.OrphanedHeaders.Num();
    
    if (Report.OrphanedCount == 0)
    {
        Report.Status = EEng_CompilationStatus::Success;
    }
    else
    {
        Report.Status = EEng_CompilationStatus::Orphaned;
    }
    
    return Report;
}

bool UEng_CompilationFixer::FixOrphanedHeaders(const TArray<FString>& HeadersToFix)
{
    if (bFixInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Compilation fix already in progress"));
        return false;
    }
    
    bFixInProgress = true;
    bool bAllFixed = true;
    
    for (const FString& HeaderName : HeadersToFix)
    {
        // Find the header path
        FString HeaderPath;
        for (const FEng_OrphanedHeader& OrphanedHeader : LastReport.OrphanedHeaders)
        {
            if (OrphanedHeader.HeaderName == HeaderName)
            {
                HeaderPath = OrphanedHeader.FilePath;
                break;
            }
        }
        
        if (!HeaderPath.IsEmpty())
        {
            if (!CreateMissingCppFile(HeaderPath))
            {
                bAllFixed = false;
                UE_LOG(LogTemp, Error, TEXT("Failed to create .cpp for: %s"), *HeaderName);
            }
        }
    }
    
    bFixInProgress = false;
    
    // Re-analyze after fixes
    LastReport = AnalyzeCompilationStatus();
    
    return bAllFixed;
}

bool UEng_CompilationFixer::CreateMissingCppFile(const FString& HeaderPath)
{
    FString CppPath = HeaderPath.Replace(TEXT(".h"), TEXT(".cpp"));
    FString HeaderName = FPaths::GetBaseFilename(HeaderPath);
    
    // Generate stub .cpp content
    FString CppContent = GenerateStubCppContent(HeaderName);
    
    // Write the .cpp file
    if (FFileHelper::SaveStringToFile(CppContent, *CppPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Created .cpp file: %s"), *CppPath);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create .cpp file: %s"), *CppPath);
        return false;
    }
}

FString UEng_CompilationFixer::GenerateStubCppContent(const FString& HeaderName)
{
    FString Content = FString::Printf(TEXT("#include \"%s.h\"\n"), *HeaderName);
    Content += TEXT("#include \"Engine/World.h\"\n");
    Content += TEXT("#include \"Engine/Engine.h\"\n");
    Content += TEXT("\n");
    
    // Add basic constructor implementation for classes that likely need it
    if (HeaderName.StartsWith(TEXT("Eng_")) || HeaderName.StartsWith(TEXT("Dir_")))
    {
        Content += FString::Printf(TEXT("// %s Implementation\n"), *HeaderName);
        Content += FString::Printf(TEXT("// Auto-generated stub by Engine Architect Compilation Fixer\n"));
        Content += TEXT("\n");
        
        // Add common constructor patterns
        if (HeaderName.Contains(TEXT("Manager")) || HeaderName.Contains(TEXT("System")))
        {
            Content += FString::Printf(TEXT("// Constructor stub for %s\n"), *HeaderName);
            Content += FString::Printf(TEXT("// TODO: Implement proper constructor logic\n"));
        }
    }
    
    Content += TEXT("\n");
    Content += FString::Printf(TEXT("// End of auto-generated stub for %s\n"), *HeaderName);
    
    return Content;
}

TArray<FString> UEng_CompilationFixer::GetCriticalOrphanedHeaders()
{
    TArray<FString> CriticalOrphaned;
    
    for (const FEng_OrphanedHeader& OrphanedHeader : LastReport.OrphanedHeaders)
    {
        if (IsHeaderCritical(OrphanedHeader.HeaderName))
        {
            CriticalOrphaned.Add(OrphanedHeader.HeaderName);
        }
    }
    
    return CriticalOrphaned;
}

bool UEng_CompilationFixer::IsHeaderCritical(const FString& HeaderName)
{
    for (const FString& CriticalSystem : CriticalSystems)
    {
        if (HeaderName.Contains(CriticalSystem))
        {
            return true;
        }
    }
    return false;
}

bool UEng_CompilationFixer::ValidateSystemIntegrity()
{
    // Check if critical classes can be loaded
    TArray<FString> CriticalClassPaths = {
        TEXT("/Script/TranspersonalGame.Eng_CriticalSystemsManager"),
        TEXT("/Script/TranspersonalGame.Eng_ArchitectureManager"),
        TEXT("/Script/TranspersonalGame.BiomeManager"),
        TEXT("/Script/TranspersonalGame.DinosaurBase")
    };
    
    bool bAllValid = true;
    
    for (const FString& ClassPath : CriticalClassPaths)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (!LoadedClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load critical class: %s"), *ClassPath);
            bAllValid = false;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Successfully validated: %s"), *ClassPath);
        }
    }
    
    return bAllValid;
}

void UEng_CompilationFixer::RunCompilationFix()
{
    UE_LOG(LogTemp, Warning, TEXT("Running Engine Architect Compilation Fix"));
    
    // Get critical orphaned headers
    TArray<FString> CriticalHeaders = GetCriticalOrphanedHeaders();
    
    if (CriticalHeaders.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fixing %d critical orphaned headers"), CriticalHeaders.Num());
        FixOrphanedHeaders(CriticalHeaders);
    }
    
    // Validate system integrity
    ValidateSystemIntegrity();
    
    LogCompilationStatus();
}

void UEng_CompilationFixer::LogCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT COMPILATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Headers: %d"), LastReport.TotalHeaders);
    UE_LOG(LogTemp, Warning, TEXT("Total CPP Files: %d"), LastReport.TotalCppFiles);
    UE_LOG(LogTemp, Warning, TEXT("Orphaned Headers: %d"), LastReport.OrphanedCount);
    
    if (LastReport.OrphanedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Critical Orphaned Headers:"));
        TArray<FString> CriticalHeaders = GetCriticalOrphanedHeaders();
        for (const FString& Header : CriticalHeaders)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Header);
        }
    }
}

// AEng_CompilationManager Implementation

AEng_CompilationManager::AEng_CompilationManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;
    
    // Set default values
    bAutoFixOnBeginPlay = true;
    FixInterval = 300.0f; // 5 minutes
    
    // Load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(CubeMeshAsset.Object);
        VisualizationMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
}

void AEng_CompilationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Manager - Begin Play"));
    
    if (bAutoFixOnBeginPlay)
    {
        AnalyzeAndFixCompilation();
    }
    
    // Set up periodic fix timer
    if (FixInterval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(FixTimerHandle, this, &AEng_CompilationManager::PerformPeriodicFix, FixInterval, true);
    }
}

void AEng_CompilationManager::AnalyzeAndFixCompilation()
{
    UEng_CompilationFixer* CompilationFixer = GetGameInstance()->GetSubsystem<UEng_CompilationFixer>();
    if (CompilationFixer)
    {
        CompilationFixer->RunCompilationFix();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Compilation Fixer subsystem"));
    }
}

void AEng_CompilationManager::CreateMissingCppFiles()
{
    UEng_CompilationFixer* CompilationFixer = GetGameInstance()->GetSubsystem<UEng_CompilationFixer>();
    if (CompilationFixer)
    {
        FEng_CompilationReport Report = CompilationFixer->AnalyzeCompilationStatus();
        
        TArray<FString> HeadersToFix;
        for (const FEng_OrphanedHeader& OrphanedHeader : Report.OrphanedHeaders)
        {
            HeadersToFix.Add(OrphanedHeader.HeaderName);
        }
        
        if (HeadersToFix.Num() > 0)
        {
            CompilationFixer->FixOrphanedHeaders(HeadersToFix);
            UE_LOG(LogTemp, Warning, TEXT("Created %d missing .cpp files"), HeadersToFix.Num());
        }
    }
}

void AEng_CompilationManager::ValidateArchitecture()
{
    UEng_CompilationFixer* CompilationFixer = GetGameInstance()->GetSubsystem<UEng_CompilationFixer>();
    if (CompilationFixer)
    {
        bool bValid = CompilationFixer->ValidateSystemIntegrity();
        UE_LOG(LogTemp, Warning, TEXT("Architecture Validation: %s"), bValid ? TEXT("PASSED") : TEXT("FAILED"));
    }
}

void AEng_CompilationManager::PerformPeriodicFix()
{
    UE_LOG(LogTemp, Warning, TEXT("Performing periodic compilation fix"));
    AnalyzeAndFixCompilation();
}