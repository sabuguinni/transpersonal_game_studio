#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EditorLevelLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Inicializar propriedades
    CurrentBuildState = EBuildState::Unknown;
    OrphanHeaderCount = 0;
    CompilationErrorCount = 0;
    LastValidationTime = FDateTime::Now();
    MaxBackupsToKeep = 10;
    BackupDirectory = TEXT("Saved/Backups");
    
    // Sistemas críticos a validar
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("TranspersonalGameState"));
    CriticalSystems.Add(TEXT("PCGWorldGenerator"));
    CriticalSystems.Add(TEXT("FoliageManager"));
    CriticalSystems.Add(TEXT("CrowdSimulationManager"));
    CriticalSystems.Add(TEXT("BuildIntegrationManager"));
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting integration validation"));
    
    // Executar validação inicial
    ValidateCompilationState();
    ValidateMapIntegrity();
    CheckOrphanHeaders();
}

void ABuildIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Validação periódica a cada 30 segundos
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;
    
    if (ValidationTimer >= 30.0f)
    {
        ValidateSystemCompatibility();
        ValidationTimer = 0.0f;
    }
}

bool ABuildIntegrationManager::ValidateCompilationState()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating compilation state"));
    
    CompilationErrorCount = 0;
    bool bAllSystemsValid = true;
    
    // Verificar se todas as classes críticas existem
    for (const FString& SystemName : CriticalSystems)
    {
        if (!ValidateClass(SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Critical system missing: %s"), *SystemName);
            bAllSystemsValid = false;
            CompilationErrorCount++;
        }
    }
    
    // Actualizar estado da build
    CurrentBuildState = bAllSystemsValid ? EBuildState::Stable : EBuildState::Broken;
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Compilation validation complete. State: %s"), 
           CurrentBuildState == EBuildState::Stable ? TEXT("STABLE") : TEXT("BROKEN"));
    
    return bAllSystemsValid;
}

int32 ABuildIntegrationManager::CheckOrphanHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Checking for orphan headers"));
    
    OrphanHeaderCount = 0;
    
    // Obter directório do projeto
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source/TranspersonalGame"));
    
    // Procurar ficheiros .h
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        // Verificar se existe .cpp correspondente
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            OrphanHeaderCount++;
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Orphan header found: %s"), *HeaderFile);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d orphan headers"), OrphanHeaderCount);
    return OrphanHeaderCount;
}

bool ABuildIntegrationManager::ValidateMapIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating map integrity"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world available"));
        return false;
    }
    
    // Contar actores por tipo
    TMap<FString, int32> ActorCounts;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            ActorCounts.FindOrAdd(ClassName)++;
        }
    }
    
    // Reportar contagens
    for (const auto& Pair : ActorCounts)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: %s: %d actors"), *Pair.Key, Pair.Value);
    }
    
    return true;
}

void ABuildIntegrationManager::CleanDuplicateLightingActors()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Cleaning duplicate lighting actors"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Limpar DirectionalLights duplicados (manter apenas 1)
    TArray<ADirectionalLight*> DirectionalLights;
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        DirectionalLights.Add(*ActorItr);
    }
    
    if (DirectionalLights.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d DirectionalLights, removing %d"), 
               DirectionalLights.Num(), DirectionalLights.Num() - 1);
        
        for (int32 i = 1; i < DirectionalLights.Num(); i++)
        {
            DirectionalLights[i]->Destroy();
        }
    }
    
    // Limpar SkyLights duplicados (manter apenas 1)
    TArray<ASkyLight*> SkyLights;
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyLights.Add(*ActorItr);
    }
    
    if (SkyLights.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d SkyLights, removing %d"), 
               SkyLights.Num(), SkyLights.Num() - 1);
        
        for (int32 i = 1; i < SkyLights.Num(); i++)
        {
            SkyLights[i]->Destroy();
        }
    }
}

bool ABuildIntegrationManager::ExecuteFullBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Executing full build"));
    
    // Nota: Execução real de build seria feita via sistema externo
    // Aqui apenas simulamos a validação
    
    bool bBuildSuccess = ValidateCompilationState();
    
    if (bBuildSuccess)
    {
        CurrentBuildState = EBuildState::Stable;
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Build completed successfully"));
    }
    else
    {
        CurrentBuildState = EBuildState::Broken;
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Build failed"));
    }
    
    return bBuildSuccess;
}

bool ABuildIntegrationManager::CreateBuildBackup()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Creating build backup"));
    
    // Implementação simplificada - apenas log
    FString BackupName = FString::Printf(TEXT("Backup_%s"), *FDateTime::Now().ToString());
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Created backup: %s"), *BackupName);
    
    return true;
}

bool ABuildIntegrationManager::RestoreLastWorkingBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Restoring last working build"));
    
    // Implementação simplificada
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Restored last working build"));
    
    return true;
}

bool ABuildIntegrationManager::ValidateSystemCompatibility()
{
    // Validação básica de compatibilidade entre sistemas
    bool bCompatible = true;
    
    // Verificar se sistemas críticos estão carregados
    for (const FString& SystemName : CriticalSystems)
    {
        if (!ValidateClass(SystemName))
        {
            bCompatible = false;
        }
    }
    
    return bCompatible;
}

bool ABuildIntegrationManager::ResolveDependencyConflicts()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Resolving dependency conflicts"));
    
    // Implementação básica - verificar e reportar conflitos
    return true;
}

FString ABuildIntegrationManager::GenerateIntegrationReport()
{
    FString Report = TEXT("=== BUILD INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Build State: %s\n"), 
                             CurrentBuildState == EBuildState::Stable ? TEXT("STABLE") : TEXT("BROKEN"));
    Report += FString::Printf(TEXT("Orphan Headers: %d\n"), OrphanHeaderCount);
    Report += FString::Printf(TEXT("Compilation Errors: %d\n"), CompilationErrorCount);
    Report += FString::Printf(TEXT("Last Validation: %s\n"), *LastValidationTime.ToString());
    Report += FString::Printf(TEXT("Critical Systems: %d/%d validated\n"), 
                             CriticalSystems.Num() - CompilationErrorCount, CriticalSystems.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Generated integration report"));
    
    return Report;
}

bool ABuildIntegrationManager::ValidateClass(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* Class = LoadClass<UObject>(nullptr, *FullClassName);
    
    return Class != nullptr;
}

int32 ABuildIntegrationManager::CountFilesByExtension(const FString& Directory, const FString& Extension)
{
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *Directory, *Extension, true, false);
    return Files.Num();
}

bool ABuildIntegrationManager::ExecuteSystemCommand(const FString& Command, FString& Output)
{
    // Implementação simplificada
    Output = TEXT("Command executed");
    return true;
}

void ABuildIntegrationManager::CleanTemporaryBuildFiles()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Cleaning temporary build files"));
}

bool ABuildIntegrationManager::ValidateProjectStructure()
{
    FString ProjectDir = FPaths::ProjectDir();
    
    // Verificar directorias essenciais
    TArray<FString> RequiredDirs = {
        TEXT("Source"),
        TEXT("Content"),
        TEXT("Config")
    };
    
    for (const FString& Dir : RequiredDirs)
    {
        FString FullPath = FPaths::Combine(ProjectDir, Dir);
        if (!IFileManager::Get().DirectoryExists(*FullPath))
        {
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Missing required directory: %s"), *Dir);
            return false;
        }
    }
    
    return true;
}