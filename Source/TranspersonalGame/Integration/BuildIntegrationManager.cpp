#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/LevelStreaming.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIntegrationActive = false;
    LastValidationTime = 0.0f;
    ValidationIntervalSeconds = 60.0f;
    bAutoCleanupDuplicates = true;
    bLogVerboseOutput = false;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialized"));
    bIntegrationActive = true;
    
    // Executar validação inicial
    FTimerHandle ValidationTimer;
    GetWorld()->GetTimerManager().SetTimer(ValidationTimer, 
        FTimerDelegate::CreateUObject(this, &UBuildIntegrationManager::PerformPeriodicValidation),
        ValidationIntervalSeconds, true);
}

void UBuildIntegrationManager::Deinitialize()
{
    bIntegrationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Deinitialized"));
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuildIntegrationManager::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    Report.ReportTimestamp = FDateTime::Now().ToString();
    
    // Gerar relatório de compilação
    GenerateCompilationReport(Report.CompilationResult);
    
    // Analisar módulos
    TArray<FString> ModuleNames = {TEXT("TranspersonalGame"), TEXT("Core"), TEXT("Engine")};
    for (const FString& ModuleName : ModuleNames)
    {
        FBuild_ModuleStatus ModuleStatus = AnalyzeModule(ModuleName);
        Report.ModuleStatuses.Add(ModuleStatus);
    }
    
    // Contar actores no mapa
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        Report.TotalActorsInMap = AllActors.Num();
        
        // Contar duplicados de lighting
        int32 DirectionalLights = CountActorsByClass(TEXT("DirectionalLight"));
        int32 SkyLights = CountActorsByClass(TEXT("SkyLight"));
        int32 HeightFogs = CountActorsByClass(TEXT("ExponentialHeightFog"));
        
        Report.DuplicateLightingActors = FMath::Max(0, DirectionalLights - 1) + 
                                        FMath::Max(0, SkyLights - 1) + 
                                        FMath::Max(0, HeightFogs - 1);
    }
    
    // Verificar se o mapa é jogável
    Report.bMapPlayable = VerifyMapPlayability();
    
    // Identificar problemas críticos
    if (Report.CompilationResult.ErrorCount > 0)
    {
        Report.CriticalIssues.Add(FString::Printf(TEXT("Compilation errors: %d"), Report.CompilationResult.ErrorCount));
    }
    
    if (Report.DuplicateLightingActors > 0)
    {
        Report.CriticalIssues.Add(FString::Printf(TEXT("Duplicate lighting actors: %d"), Report.DuplicateLightingActors));
    }
    
    TArray<FString> OrphanHeaders = GetOrphanHeaders();
    if (OrphanHeaders.Num() > 0)
    {
        Report.CriticalIssues.Add(FString::Printf(TEXT("Orphan headers: %d"), OrphanHeaders.Num()));
    }
    
    LastReport = Report;
    
    if (bLogVerboseOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration Report Generated:"));
        UE_LOG(LogTemp, Warning, TEXT("- Total Actors: %d"), Report.TotalActorsInMap);
        UE_LOG(LogTemp, Warning, TEXT("- Duplicate Lighting: %d"), Report.DuplicateLightingActors);
        UE_LOG(LogTemp, Warning, TEXT("- Critical Issues: %d"), Report.CriticalIssues.Num());
        UE_LOG(LogTemp, Warning, TEXT("- Map Playable: %s"), Report.bMapPlayable ? TEXT("Yes") : TEXT("No"));
    }
    
    return Report;
}

bool UBuildIntegrationManager::ValidateModuleIntegrity()
{
    bool bIntegrityValid = true;
    
    // Verificar pares header/implementation
    if (!CheckHeaderImplementationPairs())
    {
        bIntegrityValid = false;
        UE_LOG(LogTemp, Error, TEXT("Module integrity check failed: Missing implementations"));
    }
    
    // Verificar dependências de sistema
    if (!ValidateSystemDependencies())
    {
        bIntegrityValid = false;
        UE_LOG(LogTemp, Error, TEXT("Module integrity check failed: System dependencies"));
    }
    
    return bIntegrityValid;
}

void UBuildIntegrationManager::CleanupDuplicateActors()
{
    if (!bAutoCleanupDuplicates)
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto cleanup disabled, skipping duplicate removal"));
        return;
    }
    
    RemoveDuplicateLightingActors();
    
    UE_LOG(LogTemp, Warning, TEXT("Duplicate actor cleanup completed"));
}

bool UBuildIntegrationManager::VerifyMapPlayability()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    bool bPlayable = true;
    
    // Verificar se existe PlayerStart
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    if (PlayerStarts.Num() == 0)
    {
        bPlayable = false;
        UE_LOG(LogTemp, Error, TEXT("Map playability check failed: No PlayerStart found"));
    }
    
    // Verificar se existe terreno
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    if (Landscapes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Map playability warning: No Landscape found"));
    }
    
    // Verificar iluminação básica
    int32 DirectionalLights = CountActorsByClass(TEXT("DirectionalLight"));
    if (DirectionalLights == 0)
    {
        bPlayable = false;
        UE_LOG(LogTemp, Error, TEXT("Map playability check failed: No DirectionalLight found"));
    }
    
    return bPlayable;
}

TArray<FString> UBuildIntegrationManager::GetOrphanHeaders()
{
    TArray<FString> OrphanHeaders;
    ScanForOrphanHeaders(OrphanHeaders);
    return OrphanHeaders;
}

bool UBuildIntegrationManager::TriggerCompilationTest()
{
    // Esta função seria usada para triggerar uma compilação externa
    // Por agora, apenas simula o teste
    UE_LOG(LogTemp, Warning, TEXT("Compilation test triggered (simulated)"));
    return true;
}

void UBuildIntegrationManager::RemoveDuplicateLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Remover DirectionalLights duplicados (manter apenas 1)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    for (int32 i = 1; i < DirectionalLights.Num(); ++i)
    {
        if (DirectionalLights[i])
        {
            DirectionalLights[i]->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("Removed duplicate DirectionalLight"));
        }
    }
    
    // Remover SkyLights duplicados
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    for (int32 i = 1; i < SkyLights.Num(); ++i)
    {
        if (SkyLights[i])
        {
            SkyLights[i]->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("Removed duplicate SkyLight"));
        }
    }
    
    // Nota: ExponentialHeightFog requer uma abordagem diferente pois é um componente
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    TArray<AActor*> FogActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogActors.Add(Actor);
        }
    }
    
    for (int32 i = 1; i < FogActors.Num(); ++i)
    {
        if (FogActors[i])
        {
            FogActors[i]->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("Removed duplicate ExponentialHeightFog"));
        }
    }
}

int32 UBuildIntegrationManager::CountActorsByClass(const FString& ClassName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 Count = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(ClassName))
        {
            Count++;
        }
    }
    
    return Count;
}

void UBuildIntegrationManager::ValidateActorDistribution()
{
    ValidateBiomeActorDistribution();
}

FBuild_ModuleStatus UBuildIntegrationManager::AnalyzeModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
    Status.LastCompileTime = FDateTime::Now().ToString();
    
    // Simular contagem de headers e implementações
    // Em implementação real, isto faria scan do filesystem
    Status.HeaderCount = 25; // Placeholder
    Status.ImplementationCount = 18; // Placeholder
    
    return Status;
}

bool UBuildIntegrationManager::CheckHeaderImplementationPairs()
{
    // Implementação simplificada - em produção faria scan real do filesystem
    TArray<FString> OrphanHeaders;
    ScanForOrphanHeaders(OrphanHeaders);
    
    if (OrphanHeaders.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d orphan headers"), OrphanHeaders.Num());
        return false;
    }
    
    return true;
}

void UBuildIntegrationManager::LogCriticalIssues(const TArray<FString>& Issues)
{
    for (const FString& Issue : Issues)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUE: %s"), *Issue);
    }
}

void UBuildIntegrationManager::PerformPeriodicValidation()
{
    if (!bIntegrationActive)
    {
        return;
    }
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Executar validações periódicas
    ValidateModuleIntegrity();
    
    if (bAutoCleanupDuplicates)
    {
        CleanupDuplicateActors();
    }
    
    // Gerar relatório
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    
    if (Report.CriticalIssues.Num() > 0)
    {
        LogCriticalIssues(Report.CriticalIssues);
    }
}

bool UBuildIntegrationManager::ValidateActorIntegrity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            UE_LOG(LogTemp, Error, TEXT("Found invalid actor in world"));
            return false;
        }
    }
    
    return true;
}

bool UBuildIntegrationManager::ValidateSystemDependencies()
{
    // Verificar se sistemas críticos estão disponíveis
    bool bDependenciesValid = true;
    
    // Verificar GameMode
    if (!GetWorld()->GetAuthGameMode())
    {
        UE_LOG(LogTemp, Warning, TEXT("No GameMode set for current world"));
        bDependenciesValid = false;
    }
    
    return bDependenciesValid;
}

void UBuildIntegrationManager::GenerateCompilationReport(FBuild_CompilationResult& OutResult)
{
    OutResult.bCompilationSuccessful = true; // Assumir sucesso se o módulo está carregado
    OutResult.ErrorCount = 0;
    OutResult.WarningCount = 0;
    OutResult.BuildTimestamp = FDateTime::Now().ToString();
    OutResult.BuildDurationSeconds = 0.0f;
    
    // Em implementação real, isto leria logs de compilação
    if (FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame")))
    {
        OutResult.bCompilationSuccessful = true;
    }
    else
    {
        OutResult.bCompilationSuccessful = false;
        OutResult.ErrorCount = 1;
        OutResult.ErrorMessages.Add(TEXT("TranspersonalGame module not loaded"));
    }
}

void UBuildIntegrationManager::ScanForOrphanHeaders(TArray<FString>& OutOrphanHeaders)
{
    // Implementação simplificada - em produção faria scan real do filesystem
    // Por agora, simular alguns headers órfãos conhecidos
    OutOrphanHeaders.Empty();
    
    // Estes seriam detectados por scan real do Source/TranspersonalGame
    OutOrphanHeaders.Add(TEXT("SomeOrphanHeader.h"));
    OutOrphanHeaders.Add(TEXT("AnotherOrphanHeader.h"));
}

void UBuildIntegrationManager::ValidateBiomeActorDistribution()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Verificar se actores estão distribuídos correctamente pelos biomas
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
    
    int32 ActorsInCenter = 0;
    int32 ActorsOutsideMap = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FVector Location = Actor->GetActorLocation();
        
        // Verificar se está dentro dos limites do mapa
        if (FMath::Abs(Location.X) > 79500.0f || FMath::Abs(Location.Y) > 76500.0f)
        {
            ActorsOutsideMap++;
        }
        
        // Verificar se está no centro (savana)
        if (FMath::Abs(Location.X) < 20000.0f && FMath::Abs(Location.Y) < 20000.0f)
        {
            ActorsInCenter++;
        }
    }
    
    if (bLogVerboseOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome Distribution - Center: %d, Outside Map: %d"), 
               ActorsInCenter, ActorsOutsideMap);
    }
}