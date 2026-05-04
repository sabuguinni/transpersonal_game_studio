#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "EditorLevelLibrary.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectIterator.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    BuildProgress = 0.0f;
    CurrentBuildPhase = TEXT("Initializing");
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager initialized"));
    
    // Inicializar estado de compilação
    RefreshCompilationStatus();
    
    // Scan inicial de módulos e actores
    ScanForModules();
    ScanForActors();
    
    UpdateBuildProgress(100.0f, TEXT("Ready"));
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager deinitialized"));
    Super::Deinitialize();
}

FBuild_CompilationStatus UBuildIntegrationManager::GetCompilationStatus() const
{
    return CurrentCompilationStatus;
}

void UBuildIntegrationManager::RefreshCompilationStatus()
{
    UpdateBuildProgress(10.0f, TEXT("Checking Compilation"));
    
    CurrentCompilationStatus = FBuild_CompilationStatus();
    CurrentCompilationStatus.LastBuildTime = FDateTime::Now().ToString();
    
    // Verificar se classes críticas estão carregadas
    int32 LoadedClasses = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogBuildIntegration, Log, TEXT("Class loaded: %s"), *ClassName);
        }
        else
        {
            CurrentCompilationStatus.ErrorMessages.Add(FString::Printf(TEXT("Failed to load class: %s"), *ClassName));
            CurrentCompilationStatus.ErrorCount++;
            UE_LOG(LogBuildIntegration, Warning, TEXT("Failed to load class: %s"), *ClassName);
        }
    }
    
    CurrentCompilationStatus.bIsCompiled = (LoadedClasses == CriticalClasses.Num());
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Compilation Status: %s (%d/%d classes loaded)"), 
           CurrentCompilationStatus.bIsCompiled ? TEXT("SUCCESS") : TEXT("FAILED"),
           LoadedClasses, CriticalClasses.Num());
    
    UpdateBuildProgress(30.0f, TEXT("Compilation Check Complete"));
}

bool UBuildIntegrationManager::ValidateAllModules()
{
    UpdateBuildProgress(40.0f, TEXT("Validating Modules"));
    
    ScanForModules();
    ValidateClassLoading();
    
    bool bAllValid = true;
    for (const FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (!ModuleStatus.bIsLoaded)
        {
            bAllValid = false;
            UE_LOG(LogBuildIntegration, Error, TEXT("Module not loaded: %s"), *ModuleStatus.ModuleName);
        }
    }
    
    UpdateBuildProgress(60.0f, TEXT("Module Validation Complete"));
    return bAllValid;
}

TArray<FBuild_ModuleStatus> UBuildIntegrationManager::GetModuleStatuses() const
{
    return ModuleStatuses;
}

bool UBuildIntegrationManager::IsModuleLoaded(const FString& ModuleName) const
{
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.ModuleName == ModuleName)
        {
            return Status.bIsLoaded;
        }
    }
    return false;
}

TArray<FString> UBuildIntegrationManager::GetLoadedClasses(const FString& ModuleName) const
{
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.ModuleName == ModuleName)
        {
            return Status.LoadedClasses;
        }
    }
    return TArray<FString>();
}

TArray<FBuild_ActorInventory> UBuildIntegrationManager::GetActorInventory() const
{
    return ActorInventories;
}

int32 UBuildIntegrationManager::CleanupDuplicateActors()
{
    UpdateBuildProgress(70.0f, TEXT("Cleaning Duplicates"));
    
    int32 ActorsDestroyed = 0;
    
    if (UWorld* World = GetWorld())
    {
        // Agrupar actores de lighting por tipo
        TMap<FString, TArray<AActor*>> LightingGroups;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FString ClassName = Actor->GetClass()->GetName();
                if (LightingActorTypes.Contains(ClassName))
                {
                    if (!LightingGroups.Contains(ClassName))
                    {
                        LightingGroups.Add(ClassName, TArray<AActor*>());
                    }
                    LightingGroups[ClassName].Add(Actor);
                }
            }
        }
        
        // Remover duplicados - manter apenas o primeiro de cada tipo
        for (auto& Group : LightingGroups)
        {
            TArray<AActor*>& Actors = Group.Value;
            if (Actors.Num() > 1)
            {
                UE_LOG(LogBuildIntegration, Warning, TEXT("Found %d %s actors - keeping only 1"), 
                       Actors.Num(), *Group.Key);
                
                // Destruir todos excepto o primeiro
                for (int32 i = 1; i < Actors.Num(); i++)
                {
                    if (Actors[i])
                    {
                        Actors[i]->Destroy();
                        ActorsDestroyed++;
                        UE_LOG(LogBuildIntegration, Log, TEXT("Destroyed duplicate %s #%d"), *Group.Key, i);
                    }
                }
            }
        }
    }
    
    // Actualizar inventário após limpeza
    ScanForActors();
    
    UpdateBuildProgress(90.0f, TEXT("Cleanup Complete"));
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Cleanup complete: %d duplicate actors destroyed"), ActorsDestroyed);
    return ActorsDestroyed;
}

bool UBuildIntegrationManager::HasDuplicateLightingActors() const
{
    for (const FBuild_ActorInventory& Inventory : ActorInventories)
    {
        if (LightingActorTypes.Contains(Inventory.ActorClass) && Inventory.bHasDuplicates)
        {
            return true;
        }
    }
    return false;
}

bool UBuildIntegrationManager::ValidateSystemIntegration()
{
    UpdateBuildProgress(50.0f, TEXT("Validating Integration"));
    
    IntegrationErrors.Empty();
    
    // Verificar dependências críticas
    bool bWorldGenLoaded = IsModuleLoaded(TEXT("PCGWorldGenerator"));
    bool bFoliageLoaded = IsModuleLoaded(TEXT("FoliageManager"));
    bool bCrowdLoaded = IsModuleLoaded(TEXT("CrowdSimulationManager"));
    
    if (!bWorldGenLoaded)
    {
        IntegrationErrors.Add(TEXT("PCGWorldGenerator not loaded - world generation unavailable"));
    }
    
    if (!bFoliageLoaded)
    {
        IntegrationErrors.Add(TEXT("FoliageManager not loaded - vegetation system unavailable"));
    }
    
    if (!bCrowdLoaded)
    {
        IntegrationErrors.Add(TEXT("CrowdSimulationManager not loaded - crowd AI unavailable"));
    }
    
    // Verificar se há duplicados críticos
    if (HasDuplicateLightingActors())
    {
        IntegrationErrors.Add(TEXT("Duplicate lighting actors detected - may cause rendering issues"));
    }
    
    bool bIntegrationValid = IntegrationErrors.Num() == 0;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("System integration validation: %s (%d errors)"), 
           bIntegrationValid ? TEXT("PASS") : TEXT("FAIL"), IntegrationErrors.Num());
    
    return bIntegrationValid;
}

TArray<FString> UBuildIntegrationManager::GetIntegrationErrors() const
{
    return IntegrationErrors;
}

bool UBuildIntegrationManager::CanSystemsIntegrate(const FString& SystemA, const FString& SystemB) const
{
    // Verificar compatibilidade entre sistemas
    // Por exemplo: WorldGen deve carregar antes de Foliage
    if (SystemA == TEXT("PCGWorldGenerator") && SystemB == TEXT("FoliageManager"))
    {
        return IsModuleLoaded(TEXT("PCGWorldGenerator"));
    }
    
    if (SystemA == TEXT("FoliageManager") && SystemB == TEXT("CrowdSimulationManager"))
    {
        return IsModuleLoaded(TEXT("FoliageManager"));
    }
    
    return true; // Por defeito, assumir compatibilidade
}

void UBuildIntegrationManager::TriggerFullBuildValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Starting full build validation"));
    
    UpdateBuildProgress(0.0f, TEXT("Starting Validation"));
    
    // 1. Verificar compilação
    RefreshCompilationStatus();
    
    // 2. Validar módulos
    ValidateAllModules();
    
    // 3. Limpar duplicados
    CleanupDuplicateActors();
    
    // 4. Validar integração
    ValidateSystemIntegration();
    
    UpdateBuildProgress(100.0f, TEXT("Validation Complete"));
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Full build validation complete"));
}

float UBuildIntegrationManager::GetBuildProgress() const
{
    return BuildProgress;
}

FString UBuildIntegrationManager::GetCurrentBuildPhase() const
{
    return CurrentBuildPhase;
}

void UBuildIntegrationManager::ScanForModules()
{
    ModuleStatuses.Empty();
    
    // Verificar módulo principal TranspersonalGame
    FBuild_ModuleStatus MainModule;
    MainModule.ModuleName = TEXT("TranspersonalGame");
    MainModule.bIsLoaded = FModuleManager::Get().IsModuleLoaded("TranspersonalGame");
    
    if (MainModule.bIsLoaded)
    {
        // Contar classes carregadas
        for (const FString& ClassName : CriticalClasses)
        {
            FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
            UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
            if (LoadedClass)
            {
                MainModule.LoadedClasses.Add(ClassName);
                MainModule.ClassCount++;
            }
        }
    }
    
    ModuleStatuses.Add(MainModule);
}

void UBuildIntegrationManager::ScanForActors()
{
    ActorInventories.Empty();
    
    if (UWorld* World = GetWorld())
    {
        TMap<FString, FBuild_ActorInventory> ActorCounts;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FString ClassName = Actor->GetClass()->GetName();
                
                if (!ActorCounts.Contains(ClassName))
                {
                    FBuild_ActorInventory NewInventory;
                    NewInventory.ActorClass = ClassName;
                    ActorCounts.Add(ClassName, NewInventory);
                }
                
                ActorCounts[ClassName].Count++;
                ActorCounts[ClassName].ActorNames.Add(Actor->GetName());
                
                // Marcar como duplicado se for actor de lighting com mais de 1 instância
                if (LightingActorTypes.Contains(ClassName) && ActorCounts[ClassName].Count > 1)
                {
                    ActorCounts[ClassName].bHasDuplicates = true;
                }
            }
        }
        
        // Converter para array
        for (auto& Pair : ActorCounts)
        {
            ActorInventories.Add(Pair.Value);
        }
    }
}

void UBuildIntegrationManager::ValidateClassLoading()
{
    for (FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        if (ModuleStatus.bIsLoaded)
        {
            // Verificar se as classes críticas estão realmente acessíveis
            int32 ValidClasses = 0;
            for (const FString& ClassName : ModuleStatus.LoadedClasses)
            {
                FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
                UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
                if (LoadedClass)
                {
                    ValidClasses++;
                }
            }
            
            if (ValidClasses != ModuleStatus.LoadedClasses.Num())
            {
                UE_LOG(LogBuildIntegration, Warning, TEXT("Module %s: %d/%d classes accessible"), 
                       *ModuleStatus.ModuleName, ValidClasses, ModuleStatus.LoadedClasses.Num());
            }
        }
    }
}

void UBuildIntegrationManager::CheckForDuplicates()
{
    for (FBuild_ActorInventory& Inventory : ActorInventories)
    {
        if (LightingActorTypes.Contains(Inventory.ActorClass) && Inventory.Count > 1)
        {
            Inventory.bHasDuplicates = true;
            UE_LOG(LogBuildIntegration, Warning, TEXT("Duplicate %s detected: %d instances"), 
                   *Inventory.ActorClass, Inventory.Count);
        }
    }
}

void UBuildIntegrationManager::UpdateBuildProgress(float Progress, const FString& Phase)
{
    BuildProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
    CurrentBuildPhase = Phase;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Build Progress: %.1f%% - %s"), BuildProgress, *CurrentBuildPhase);
}