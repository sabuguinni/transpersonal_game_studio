#include "QA_ValidationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

AQA_ValidationManager::AQA_ValidationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bAutoRunOnBeginPlay = false;
    ValidationInterval = 60.0f; // 1 minuto
    bLogToConsole = true;
    bLogToFile = true;
    LastValidationTime = 0.0f;
}

void AQA_ValidationManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        // Delay inicial para permitir que outros sistemas inicializem
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &AQA_ValidationManager::RunAllValidationTests,
            2.0f,
            false
        );
    }
}

void AQA_ValidationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (ValidationInterval > 0.0f)
    {
        LastValidationTime += DeltaTime;
        if (LastValidationTime >= ValidationInterval)
        {
            RunAllValidationTests();
            LastValidationTime = 0.0f;
        }
    }
}

void AQA_ValidationManager::RunAllValidationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_ValidationManager: Iniciando testes de validação completos"));
    
    ClearValidationReports();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Executar todos os testes
    ValidateMapStructure();
    ValidateLightingSetup();
    ValidatePlayerSystems();
    ValidateVFXSystems();
    ValidateBiomeDistribution();
    
    float TotalTime = FPlatformTime::Seconds() - StartTime;
    
    // Resumo final
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    int32 CriticalCount = 0;
    
    for (const FQA_ValidationReport& Report : ValidationReports)
    {
        switch (Report.Result)
        {
            case EQA_ValidationResult::Pass: PassCount++; break;
            case EQA_ValidationResult::Warning: WarningCount++; break;
            case EQA_ValidationResult::Fail: FailCount++; break;
            case EQA_ValidationResult::Critical: CriticalCount++; break;
        }
    }
    
    FString SummaryMessage = FString::Printf(
        TEXT("Validação completa: %d PASS, %d WARNING, %d FAIL, %d CRITICAL (%.2fs)"),
        PassCount, WarningCount, FailCount, CriticalCount, TotalTime
    );
    
    AddValidationReport(TEXT("SUMMARY"), 
        CriticalCount > 0 ? EQA_ValidationResult::Critical : 
        FailCount > 0 ? EQA_ValidationResult::Fail :
        WarningCount > 0 ? EQA_ValidationResult::Warning : EQA_ValidationResult::Pass,
        SummaryMessage, TotalTime);
    
    if (bLogToFile)
    {
        ExportReportsToFile();
    }
}

void AQA_ValidationManager::ValidateMapStructure()
{
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("MapStructure"), EQA_ValidationResult::Critical, 
            TEXT("Mundo não encontrado"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    // Contar todos os actores
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    if (TotalActors < 10)
    {
        AddValidationReport(TEXT("MapStructure"), EQA_ValidationResult::Fail,
            FString::Printf(TEXT("Poucos actores no mapa: %d (mínimo esperado: 10)"), TotalActors),
            FPlatformTime::Seconds() - StartTime);
    }
    else if (TotalActors > 1000)
    {
        AddValidationReport(TEXT("MapStructure"), EQA_ValidationResult::Warning,
            FString::Printf(TEXT("Muitos actores no mapa: %d (pode afectar performance)"), TotalActors),
            FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationReport(TEXT("MapStructure"), EQA_ValidationResult::Pass,
            FString::Printf(TEXT("Número adequado de actores: %d"), TotalActors),
            FPlatformTime::Seconds() - StartTime);
    }
}

void AQA_ValidationManager::ValidateLightingSetup()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Validar DirectionalLights
    if (!ValidateActorCount(ADirectionalLight::StaticClass(), 1, 1, TEXT("DirectionalLight")))
    {
        AddValidationReport(TEXT("Lighting"), EQA_ValidationResult::Fail,
            TEXT("DirectionalLight: deve existir exactamente 1"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    // Validar SkyLight (através de componente)
    int32 SkyLightCount = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->FindComponentByClass<USkyLightComponent>())
        {
            SkyLightCount++;
        }
    }
    
    if (SkyLightCount != 1)
    {
        AddValidationReport(TEXT("Lighting"), EQA_ValidationResult::Warning,
            FString::Printf(TEXT("SkyLight: encontrados %d, esperado 1"), SkyLightCount),
            FPlatformTime::Seconds() - StartTime);
    }
    
    // Validar ExponentialHeightFog
    int32 FogCount = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogCount++;
        }
    }
    
    if (FogCount > 1)
    {
        AddValidationReport(TEXT("Lighting"), EQA_ValidationResult::Warning,
            FString::Printf(TEXT("ExponentialHeightFog: encontrados %d, esperado 1"), FogCount),
            FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationReport(TEXT("Lighting"), EQA_ValidationResult::Pass,
            TEXT("Setup de iluminação validado"), FPlatformTime::Seconds() - StartTime);
    }
}

void AQA_ValidationManager::ValidatePlayerSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Validar PlayerStart
    if (!ValidateActorCount(APlayerStart::StaticClass(), 1, 5, TEXT("PlayerStart")))
    {
        AddValidationReport(TEXT("PlayerSystems"), EQA_ValidationResult::Fail,
            TEXT("PlayerStart: deve existir pelo menos 1"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    AddValidationReport(TEXT("PlayerSystems"), EQA_ValidationResult::Pass,
        TEXT("Sistemas de jogador validados"), FPlatformTime::Seconds() - StartTime);
}

void AQA_ValidationManager::ValidateVFXSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Procurar actores VFX
    int32 VFXActorCount = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        FString ClassName = ActorItr->GetClass()->GetName();
        if (ClassName.Contains(TEXT("VFX")))
        {
            VFXActorCount++;
        }
    }
    
    if (VFXActorCount == 0)
    {
        AddValidationReport(TEXT("VFXSystems"), EQA_ValidationResult::Warning,
            TEXT("Nenhum actor VFX encontrado no mapa"), FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationReport(TEXT("VFXSystems"), EQA_ValidationResult::Pass,
            FString::Printf(TEXT("Encontrados %d actores VFX"), VFXActorCount),
            FPlatformTime::Seconds() - StartTime);
    }
}

void AQA_ValidationManager::ValidateBiomeDistribution()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Definir zonas dos biomas (conforme especificação)
    struct FBiomeZone
    {
        FString Name;
        FVector Center;
        FVector MinBounds;
        FVector MaxBounds;
        int32 ActorCount;
    };
    
    TArray<FBiomeZone> Biomes = {
        {TEXT("Pantano"), FVector(-50000, -45000, 0), FVector(-77500, -76500, -1000), FVector(-25000, -15000, 1000), 0},
        {TEXT("Floresta"), FVector(-45000, 40000, 0), FVector(-77500, 15000, -1000), FVector(-15000, 76500, 1000), 0},
        {TEXT("Savana"), FVector(0, 0, 0), FVector(-20000, -20000, -1000), FVector(20000, 20000, 1000), 0},
        {TEXT("Deserto"), FVector(55000, 0, 0), FVector(25000, -30000, -1000), FVector(79500, 30000, 1000), 0},
        {TEXT("Montanha"), FVector(40000, 50000, 500), FVector(15000, 20000, -1000), FVector(79500, 76500, 2000), 0}
    };
    
    // Contar actores em cada bioma
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        FVector ActorLocation = ActorItr->GetActorLocation();
        
        for (FBiomeZone& Biome : Biomes)
        {
            if (ActorLocation.X >= Biome.MinBounds.X && ActorLocation.X <= Biome.MaxBounds.X &&
                ActorLocation.Y >= Biome.MinBounds.Y && ActorLocation.Y <= Biome.MaxBounds.Y &&
                ActorLocation.Z >= Biome.MinBounds.Z && ActorLocation.Z <= Biome.MaxBounds.Z)
            {
                Biome.ActorCount++;
                break;
            }
        }
    }
    
    // Validar distribuição
    bool bDistributionOK = true;
    for (const FBiomeZone& Biome : Biomes)
    {
        if (Biome.ActorCount == 0)
        {
            AddValidationReport(TEXT("BiomeDistribution"), EQA_ValidationResult::Warning,
                FString::Printf(TEXT("Bioma %s vazio (0 actores)"), *Biome.Name),
                FPlatformTime::Seconds() - StartTime);
            bDistributionOK = false;
        }
    }
    
    if (bDistributionOK)
    {
        AddValidationReport(TEXT("BiomeDistribution"), EQA_ValidationResult::Pass,
            TEXT("Distribuição de biomas validada"), FPlatformTime::Seconds() - StartTime);
    }
}

TArray<FQA_ValidationReport> AQA_ValidationManager::GetValidationReports() const
{
    return ValidationReports;
}

void AQA_ValidationManager::ClearValidationReports()
{
    ValidationReports.Empty();
}

void AQA_ValidationManager::ExportReportsToFile()
{
    FString ReportContent = TEXT("=== QA VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    for (const FQA_ValidationReport& Report : ValidationReports)
    {
        FString ResultString;
        switch (Report.Result)
        {
            case EQA_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
            case EQA_ValidationResult::Warning: ResultString = TEXT("WARNING"); break;
            case EQA_ValidationResult::Fail: ResultString = TEXT("FAIL"); break;
            case EQA_ValidationResult::Critical: ResultString = TEXT("CRITICAL"); break;
        }
        
        ReportContent += FString::Printf(TEXT("[%s] %s: %s (%.3fs)\n"),
            *ResultString, *Report.TestName, *Report.Message, Report.ExecutionTime);
    }
    
    FString FilePath = FPaths::ProjectLogDir() / TEXT("QA_ValidationReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Report exportado para: %s"), *FilePath);
}

void AQA_ValidationManager::AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime)
{
    FQA_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    
    if (bLogToConsole)
    {
        LogValidationResult(Report);
    }
}

void AQA_ValidationManager::LogValidationResult(const FQA_ValidationReport& Report)
{
    FString ResultString;
    switch (Report.Result)
    {
        case EQA_ValidationResult::Pass: 
            UE_LOG(LogTemp, Log, TEXT("QA PASS [%s]: %s"), *Report.TestName, *Report.Message);
            break;
        case EQA_ValidationResult::Warning:
            UE_LOG(LogTemp, Warning, TEXT("QA WARNING [%s]: %s"), *Report.TestName, *Report.Message);
            break;
        case EQA_ValidationResult::Fail:
            UE_LOG(LogTemp, Error, TEXT("QA FAIL [%s]: %s"), *Report.TestName, *Report.Message);
            break;
        case EQA_ValidationResult::Critical:
            UE_LOG(LogTemp, Fatal, TEXT("QA CRITICAL [%s]: %s"), *Report.TestName, *Report.Message);
            break;
    }
}

int32 AQA_ValidationManager::CountActorsOfClass(UClass* ActorClass)
{
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld(), ActorClass); ActorItr; ++ActorItr)
    {
        Count++;
    }
    return Count;
}

bool AQA_ValidationManager::ValidateActorCount(UClass* ActorClass, int32 ExpectedMin, int32 ExpectedMax, const FString& ActorTypeName)
{
    int32 ActualCount = CountActorsOfClass(ActorClass);
    return ActualCount >= ExpectedMin && ActualCount <= ExpectedMax;
}