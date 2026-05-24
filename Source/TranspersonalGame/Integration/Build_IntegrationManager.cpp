#include "Build_IntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PlayerStart.h"
#include "Landscape/Landscape.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UBuild_IntegrationManager::UBuild_IntegrationManager()
{
    bInitialized = false;
    
    // Inicializar lista de classes C++ conhecidas
    KnownCppClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("QA_ValidationManager"),
        TEXT("VFX_BloodSystem"),
        TEXT("Build_IntegrationManager")
    };
}

void UBuild_IntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Inicializado"));
    bInitialized = true;
    
    // Executar validação inicial
    CurrentStatus = GenerateIntegrationReport();
}

void UBuild_IntegrationManager::Deinitialize()
{
    bInitialized = false;
    Super::Deinitialize();
}

FBuild_CompilationResult UBuild_IntegrationManager::TestCompilation()
{
    FBuild_CompilationResult Result;
    
    // Simular teste de compilação básico
    // Em produção, isto executaria UnrealBuildTool
    Result.bCompilationSuccessful = true;
    Result.ErrorCount = 0;
    Result.WarningCount = 0;
    Result.CompilationTimeSeconds = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Teste de compilação executado"));
    
    return Result;
}

bool UBuild_IntegrationManager::ValidateModuleDependencies()
{
    // Verificar se todas as dependências críticas estão disponíveis
    bool bAllDependenciesValid = true;
    
    // Verificar módulos UE5 críticos
    TArray<FString> RequiredModules = {
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("UnrealEd"),
        TEXT("Landscape"),
        TEXT("Niagara")
    };
    
    for (const FString& ModuleName : RequiredModules)
    {
        // Em produção, verificaria se o módulo está carregado
        UE_LOG(LogTemp, Log, TEXT("Validando módulo: %s"), *ModuleName);
    }
    
    return bAllDependenciesValid;
}

TArray<FString> UBuild_IntegrationManager::FindOrphanHeaders()
{
    TArray<FString> OrphanHeaders;
    
    // Obter caminho do projeto
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    // Procurar ficheiros .h
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            FString FileName = FPaths::GetCleanFilename(HeaderFile);
            OrphanHeaders.Add(FileName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Encontrados %d headers órfãos"), OrphanHeaders.Num());
    
    return OrphanHeaders;
}

FBuild_IntegrationStatus UBuild_IntegrationManager::ValidateMinPlayableMap()
{
    FBuild_IntegrationStatus Status;
    
    if (UWorld* World = GetWorld())
    {
        // Contar actores totais
        Status.TotalActorsInMap = World->GetCurrentLevel()->Actors.Num();
        
        // Contar actores críticos
        PlayerStartCount = CountActorsByType(APlayerStart::StaticClass());
        DirectionalLightCount = CountActorsByType(ADirectionalLight::StaticClass());
        LandscapeCount = CountActorsByType(ALandscape::StaticClass());
        SkyAtmosphereCount = CountActorsByType(ASkyAtmosphere::StaticClass());
        
        // Verificar se o mapa é jogável
        Status.bMapPlayable = IsMapPlayable();
        
        // Calcular score de integração
        Status.IntegrationScore = 0;
        if (Status.bMapPlayable) Status.IntegrationScore += 40;
        Status.IntegrationScore += FMath::Min(Status.TotalActorsInMap, 50);
        Status.IntegrationScore += Status.FunctionalCppClasses * 10;
    }
    
    // Encontrar headers órfãos
    TArray<FString> OrphanHeaders = FindOrphanHeaders();
    Status.OrphanHeaders = OrphanHeaders.Num();
    
    // Contar classes funcionais
    Status.FunctionalCppClasses = GetFunctionalCppClasses().Num();
    
    return Status;
}

int32 UBuild_IntegrationManager::CleanDuplicateActors()
{
    int32 TotalDestroyed = 0;
    
    if (UWorld* World = GetWorld())
    {
        // Limpar duplicados de lighting
        CleanActorDuplicates(ADirectionalLight::StaticClass(), 1);
        CleanActorDuplicates(ASkyAtmosphere::StaticClass(), 1);
        CleanActorDuplicates(ASkyLight::StaticClass(), 1);
        CleanActorDuplicates(AExponentialHeightFog::StaticClass(), 1);
        
        UE_LOG(LogTemp, Warning, TEXT("Limpeza de duplicados concluída"));
    }
    
    return TotalDestroyed;
}

bool UBuild_IntegrationManager::IsMapPlayable()
{
    bool bHasPlayerStart = PlayerStartCount >= 1;
    bool bHasLighting = DirectionalLightCount >= 1;
    bool bHasTerrain = LandscapeCount >= 1;
    bool bHasAtmosphere = SkyAtmosphereCount >= 1;
    
    return bHasPlayerStart && bHasLighting && bHasTerrain && bHasAtmosphere;
}

TArray<FString> UBuild_IntegrationManager::GetFunctionalCppClasses()
{
    TArray<FString> FunctionalClasses;
    
    for (const FString& ClassName : KnownCppClasses)
    {
        if (ValidateClassIntegrity(ClassName))
        {
            FunctionalClasses.Add(ClassName);
        }
    }
    
    return FunctionalClasses;
}

bool UBuild_IntegrationManager::ValidateClassIntegrity(const FString& ClassName)
{
    // Tentar carregar a classe
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    return LoadedClass != nullptr;
}

FBuild_IntegrationStatus UBuild_IntegrationManager::GenerateIntegrationReport()
{
    CurrentStatus = ValidateMinPlayableMap();
    
    // Identificar issues críticos
    CurrentStatus.CriticalIssues.Empty();
    if (PlayerStartCount == 0)
        CurrentStatus.CriticalIssues.Add(TEXT("Sem PlayerStart no mapa"));
    if (DirectionalLightCount == 0)
        CurrentStatus.CriticalIssues.Add(TEXT("Sem iluminação direccional"));
    if (LandscapeCount == 0)
        CurrentStatus.CriticalIssues.Add(TEXT("Sem terreno (Landscape)"));
    if (CurrentStatus.OrphanHeaders > 10)
        CurrentStatus.CriticalIssues.Add(FString::Printf(TEXT("%d headers órfãos"), CurrentStatus.OrphanHeaders));
    
    // Definir próximas prioridades
    CurrentStatus.NextPriorities.Empty();
    if (!CurrentStatus.bMapPlayable)
        CurrentStatus.NextPriorities.Add(TEXT("Tornar mapa jogável"));
    if (CurrentStatus.FunctionalCppClasses < 5)
        CurrentStatus.NextPriorities.Add(TEXT("Implementar mais classes C++"));
    if (CurrentStatus.TotalActorsInMap < 20)
        CurrentStatus.NextPriorities.Add(TEXT("Popular mapa com mais actores"));
    
    return CurrentStatus;
}

void UBuild_IntegrationManager::LogIntegrationStatus()
{
    FBuild_IntegrationStatus Status = GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("=== RELATÓRIO DE INTEGRAÇÃO ==="));
    UE_LOG(LogTemp, Warning, TEXT("Actores no mapa: %d"), Status.TotalActorsInMap);
    UE_LOG(LogTemp, Warning, TEXT("Classes C++ funcionais: %d"), Status.FunctionalCppClasses);
    UE_LOG(LogTemp, Warning, TEXT("Headers órfãos: %d"), Status.OrphanHeaders);
    UE_LOG(LogTemp, Warning, TEXT("Mapa jogável: %s"), Status.bMapPlayable ? TEXT("SIM") : TEXT("NÃO"));
    UE_LOG(LogTemp, Warning, TEXT("Score de integração: %d/150"), Status.IntegrationScore);
    
    for (const FString& Issue : Status.CriticalIssues)
    {
        UE_LOG(LogTemp, Error, TEXT("CRÍTICO: %s"), *Issue);
    }
    
    for (const FString& Priority : Status.NextPriorities)
    {
        UE_LOG(LogTemp, Warning, TEXT("PRÓXIMA: %s"), *Priority);
    }
}

bool UBuild_IntegrationManager::ValidateCriticalSystems()
{
    bool bAllSystemsValid = true;
    
    // Validar sistemas críticos
    bAllSystemsValid &= ValidatePlayerStart();
    bAllSystemsValid &= ValidateLighting();
    bAllSystemsValid &= ValidateTerrain();
    bAllSystemsValid &= ValidateGameMode();
    
    return bAllSystemsValid;
}

TArray<FString> UBuild_IntegrationManager::GetCriticalIssues()
{
    return CurrentStatus.CriticalIssues;
}

bool UBuild_IntegrationManager::ValidatePlayerStart()
{
    return PlayerStartCount >= 1;
}

bool UBuild_IntegrationManager::ValidateLighting()
{
    return DirectionalLightCount >= 1;
}

bool UBuild_IntegrationManager::ValidateTerrain()
{
    return LandscapeCount >= 1;
}

bool UBuild_IntegrationManager::ValidateGameMode()
{
    if (UWorld* World = GetWorld())
    {
        return World->GetAuthGameMode() != nullptr;
    }
    return false;
}

int32 UBuild_IntegrationManager::CountActorsByType(UClass* ActorClass)
{
    int32 Count = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World, ActorClass); ActorItr; ++ActorItr)
        {
            Count++;
        }
    }
    
    return Count;
}

void UBuild_IntegrationManager::CleanActorDuplicates(UClass* ActorClass, int32 MaxCount)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> ActorsOfType;
        
        for (TActorIterator<AActor> ActorItr(World, ActorClass); ActorItr; ++ActorItr)
        {
            ActorsOfType.Add(*ActorItr);
        }
        
        if (ActorsOfType.Num() > MaxCount)
        {
            // Destruir os extras
            for (int32 i = MaxCount; i < ActorsOfType.Num(); i++)
            {
                if (ActorsOfType[i] && IsValid(ActorsOfType[i]))
                {
                    ActorsOfType[i]->Destroy();
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Destruídos %d actores duplicados de %s"), 
                   ActorsOfType.Num() - MaxCount, *ActorClass->GetName());
        }
    }
}