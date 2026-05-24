#include "World_SeasonalSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Landscape/LandscapeComponent.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"

UWorld_SeasonalSystem::UWorld_SeasonalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    SetupDefaultSeasonalParameters();
}

void UWorld_SeasonalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEnvironmentReferences();
    SetupDefaultSeasonalParameters();
    ApplySeasonalChanges();
    
    UE_LOG(LogTemp, Warning, TEXT("World_SeasonalSystem: Initialized with season %d"), (int32)CurrentSeason);
}

void UWorld_SeasonalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoProgressSeasons)
    {
        UpdateSeasonProgress(DeltaTime);
    }
    
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime * TransitionSpeed;
        float TransitionAlpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        CurrentParameters = LerpSeasonalParameters(PreviousParameters, TargetParameters, TransitionAlpha);
        ApplySeasonalChanges();
        
        if (TransitionAlpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
            UE_LOG(LogTemp, Log, TEXT("World_SeasonalSystem: Season transition completed"));
        }
    }
    else
    {
        InterpolateSeasonalParameters();
    }
}

void UWorld_SeasonalSystem::SetSeason(EWorld_Season NewSeason)
{
    if (NewSeason != CurrentSeason)
    {
        PreviousParameters = CurrentParameters;
        CurrentSeason = NewSeason;
        TargetParameters = GetSeasonParameters(NewSeason);
        
        bIsTransitioning = true;
        TransitionTimer = 0.0f;
        SeasonProgress = 0.0f;
        SeasonTimer = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("World_SeasonalSystem: Transitioning to season %d"), (int32)NewSeason);
    }
}

void UWorld_SeasonalSystem::AdvanceToNextSeason()
{
    EWorld_Season NextSeason;
    switch (CurrentSeason)
    {
        case EWorld_Season::DrySeasonEarly:
            NextSeason = EWorld_Season::DrySeasonPeak;
            break;
        case EWorld_Season::DrySeasonPeak:
            NextSeason = EWorld_Season::WetSeasonEarly;
            break;
        case EWorld_Season::WetSeasonEarly:
            NextSeason = EWorld_Season::WetSeasonPeak;
            break;
        case EWorld_Season::WetSeasonPeak:
            NextSeason = EWorld_Season::DrySeasonEarly;
            break;
        default:
            NextSeason = EWorld_Season::DrySeasonEarly;
            break;
    }
    
    SetSeason(NextSeason);
}

void UWorld_SeasonalSystem::ApplySeasonalChanges()
{
    UpdateLighting();
    UpdateAtmosphere();
    UpdateVegetation();
    UpdateWaterLevels();
}

void UWorld_SeasonalSystem::InitializeEnvironmentReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find the main directional light (sun)
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            ADirectionalLight* Light = Cast<ADirectionalLight>(Actor);
            if (Light && Light->GetActorNameOrLabel().Contains(TEXT("Sun")))
            {
                SunLight = Light;
                break;
            }
        }
        
        if (!SunLight && FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }
    
    // Find the main landscape
    if (!MainLandscape)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            MainLandscape = Cast<ALandscape>(FoundActors[0]);
        }
    }
    
    // Load environment material parameter collection
    if (!EnvironmentMPC)
    {
        EnvironmentMPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Materials/MPC_Environment"));
        if (!EnvironmentMPC)
        {
            UE_LOG(LogTemp, Warning, TEXT("World_SeasonalSystem: Could not find MPC_Environment, creating runtime parameters"));
        }
    }
}

void UWorld_SeasonalSystem::UpdateLighting()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        
        LightComp->SetIntensity(CurrentParameters.SunIntensity);
        LightComp->SetLightColor(CurrentParameters.SunColor);
        
        // Adjust sun angle based on season (Cretaceous had more consistent daylight)
        FRotator CurrentRotation = SunLight->GetActorRotation();
        float SeasonalAngleOffset = 0.0f;
        
        switch (CurrentSeason)
        {
            case EWorld_Season::DrySeasonEarly:
                SeasonalAngleOffset = -5.0f; // Slightly lower sun
                break;
            case EWorld_Season::DrySeasonPeak:
                SeasonalAngleOffset = -10.0f; // Lower sun, harsher light
                break;
            case EWorld_Season::WetSeasonEarly:
                SeasonalAngleOffset = 5.0f; // Higher sun
                break;
            case EWorld_Season::WetSeasonPeak:
                SeasonalAngleOffset = 10.0f; // Highest sun, diffused by clouds
                break;
        }
        
        CurrentRotation.Pitch = -45.0f + SeasonalAngleOffset;
        SunLight->SetActorRotation(CurrentRotation);
    }
}

void UWorld_SeasonalSystem::UpdateAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Update material parameter collection if available
    if (EnvironmentMPC)
    {
        UMaterialParameterCollectionInstance* MPCInstance = World->GetParameterCollectionInstance(EnvironmentMPC);
        if (MPCInstance)
        {
            MPCInstance->SetScalarParameterValue(FName("Humidity"), CurrentParameters.AtmosphereHumidity);
            MPCInstance->SetScalarParameterValue(FName("CloudCoverage"), CurrentParameters.CloudCoverage);
            MPCInstance->SetScalarParameterValue(FName("Temperature"), CurrentParameters.AverageTemperature);
            MPCInstance->SetVectorParameterValue(FName("AtmosphereTint"), CurrentParameters.SunColor);
        }
    }
    
    // Find and update sky atmosphere component
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        USkyAtmosphereComponent* SkyComp = Actor->FindComponentByClass<USkyAtmosphereComponent>();
        if (SkyComp)
        {
            // Adjust atmosphere parameters based on season
            float HazeMultiplier = 1.0f + (CurrentParameters.AtmosphereHumidity * 0.5f);
            SkyComp->MultiScatteringFactor = HazeMultiplier;
            
            // Adjust ground albedo based on vegetation
            FLinearColor GroundAlbedo = FLinearColor(0.3f, 0.25f, 0.2f) * CurrentParameters.VegetationDensity;
            SkyComp->GroundAlbedo = GroundAlbedo;
            
            break;
        }
    }
}

void UWorld_SeasonalSystem::UpdateVegetation()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Update foliage density and tinting
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AInstancedFoliageActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        AInstancedFoliageActor* FoliageActor = Cast<AInstancedFoliageActor>(Actor);
        if (FoliageActor)
        {
            // Get all foliage components and update their parameters
            TArray<UActorComponent*> FoliageComponents = FoliageActor->GetInstanceComponents().Array();
            
            for (UActorComponent* Component : FoliageComponents)
            {
                UFoliageInstancedStaticMeshComponent* FoliageComp = Cast<UFoliageInstancedStaticMeshComponent>(Component);
                if (FoliageComp)
                {
                    // Adjust instance density based on season
                    float DensityMultiplier = CurrentParameters.VegetationDensity;
                    
                    // In dry seasons, reduce visible instances
                    if (CurrentSeason == EWorld_Season::DrySeasonPeak)
                    {
                        DensityMultiplier *= 0.7f;
                    }
                    
                    // Update material parameters for vegetation tinting
                    UMaterialInterface* Material = FoliageComp->GetMaterial(0);
                    if (Material)
                    {
                        UMaterialInstanceDynamic* DynamicMaterial = FoliageComp->CreateAndSetMaterialInstanceDynamic(0);
                        if (DynamicMaterial)
                        {
                            DynamicMaterial->SetVectorParameterValue(FName("SeasonalTint"), CurrentParameters.VegetationTint);
                            DynamicMaterial->SetScalarParameterValue(FName("HealthMultiplier"), CurrentParameters.VegetationDensity);
                        }
                    }
                }
            }
        }
    }
}

void UWorld_SeasonalSystem::UpdateWaterLevels()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find water bodies and adjust their levels
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor->GetActorNameOrLabel().Contains(TEXT("Water")) || 
            Actor->GetActorNameOrLabel().Contains(TEXT("River")) ||
            Actor->GetActorNameOrLabel().Contains(TEXT("Lake")))
        {
            FVector CurrentLocation = Actor->GetActorLocation();
            CurrentLocation.Z = CurrentParameters.WaterLevel;
            Actor->SetActorLocation(CurrentLocation);
            
            // Update water material parameters
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                UMaterialInterface* Material = MeshComp->GetMaterial(0);
                if (Material)
                {
                    UMaterialInstanceDynamic* DynamicMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
                    if (DynamicMaterial)
                    {
                        float WaterClarity = 1.0f - (CurrentParameters.AtmosphereHumidity * 0.3f);
                        DynamicMaterial->SetScalarParameterValue(FName("WaterClarity"), WaterClarity);
                        DynamicMaterial->SetScalarParameterValue(FName("WaterLevel"), CurrentParameters.WaterLevel);
                    }
                }
            }
        }
    }
}

void UWorld_SeasonalSystem::UpdateSeasonProgress(float DeltaTime)
{
    SeasonTimer += DeltaTime;
    SeasonProgress = FMath::Clamp(SeasonTimer / SeasonDuration, 0.0f, 1.0f);
    
    if (SeasonProgress >= 1.0f)
    {
        AdvanceToNextSeason();
    }
}

void UWorld_SeasonalSystem::InterpolateSeasonalParameters()
{
    if (!bIsTransitioning)
    {
        CurrentParameters = GetSeasonParameters(CurrentSeason);
        
        // Apply subtle variations based on season progress
        float ProgressVariation = FMath::Sin(SeasonProgress * PI) * 0.1f;
        CurrentParameters.SunIntensity *= (1.0f + ProgressVariation);
        CurrentParameters.AtmosphereHumidity += ProgressVariation * 0.1f;
    }
}

FWorld_SeasonalParameters UWorld_SeasonalSystem::GetSeasonParameters(EWorld_Season Season) const
{
    switch (Season)
    {
        case EWorld_Season::DrySeasonEarly:
            return DrySeasonEarlyParams;
        case EWorld_Season::DrySeasonPeak:
            return DrySeasonPeakParams;
        case EWorld_Season::WetSeasonEarly:
            return WetSeasonEarlyParams;
        case EWorld_Season::WetSeasonPeak:
            return WetSeasonPeakParams;
        default:
            return DrySeasonEarlyParams;
    }
}

FWorld_SeasonalParameters UWorld_SeasonalSystem::LerpSeasonalParameters(const FWorld_SeasonalParameters& A, const FWorld_SeasonalParameters& B, float Alpha) const
{
    FWorld_SeasonalParameters Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.AtmosphereHumidity = FMath::Lerp(A.AtmosphereHumidity, B.AtmosphereHumidity, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.VegetationDensity = FMath::Lerp(A.VegetationDensity, B.VegetationDensity, Alpha);
    Result.VegetationTint = FMath::Lerp(A.VegetationTint, B.VegetationTint, Alpha);
    Result.WaterLevel = FMath::Lerp(A.WaterLevel, B.WaterLevel, Alpha);
    Result.AverageTemperature = FMath::Lerp(A.AverageTemperature, B.AverageTemperature, Alpha);
    Result.RainfallProbability = FMath::Lerp(A.RainfallProbability, B.RainfallProbability, Alpha);
    
    return Result;
}

void UWorld_SeasonalSystem::SetupDefaultSeasonalParameters()
{
    // Early Dry Season - Transition from wet to dry
    DrySeasonEarlyParams.SunIntensity = 3.5f;
    DrySeasonEarlyParams.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    DrySeasonEarlyParams.AtmosphereHumidity = 0.4f;
    DrySeasonEarlyParams.CloudCoverage = 0.2f;
    DrySeasonEarlyParams.VegetationDensity = 0.9f;
    DrySeasonEarlyParams.VegetationTint = FLinearColor(0.8f, 1.0f, 0.7f, 1.0f);
    DrySeasonEarlyParams.WaterLevel = -5.0f;
    DrySeasonEarlyParams.AverageTemperature = 32.0f;
    DrySeasonEarlyParams.RainfallProbability = 0.1f;
    
    // Peak Dry Season - Harshest conditions
    DrySeasonPeakParams.SunIntensity = 4.5f;
    DrySeasonPeakParams.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DrySeasonPeakParams.AtmosphereHumidity = 0.2f;
    DrySeasonPeakParams.CloudCoverage = 0.1f;
    DrySeasonPeakParams.VegetationDensity = 0.6f;
    DrySeasonPeakParams.VegetationTint = FLinearColor(0.7f, 0.8f, 0.5f, 1.0f);
    DrySeasonPeakParams.WaterLevel = -15.0f;
    DrySeasonPeakParams.AverageTemperature = 38.0f;
    DrySeasonPeakParams.RainfallProbability = 0.05f;
    
    // Early Wet Season - Recovery begins
    WetSeasonEarlyParams.SunIntensity = 3.0f;
    WetSeasonEarlyParams.SunColor = FLinearColor(0.95f, 1.0f, 1.0f, 1.0f);
    WetSeasonEarlyParams.AtmosphereHumidity = 0.7f;
    WetSeasonEarlyParams.CloudCoverage = 0.5f;
    WetSeasonEarlyParams.VegetationDensity = 1.1f;
    WetSeasonEarlyParams.VegetationTint = FLinearColor(0.6f, 1.0f, 0.8f, 1.0f);
    WetSeasonEarlyParams.WaterLevel = 5.0f;
    WetSeasonEarlyParams.AverageTemperature = 28.0f;
    WetSeasonEarlyParams.RainfallProbability = 0.4f;
    
    // Peak Wet Season - Lush and humid
    WetSeasonPeakParams.SunIntensity = 2.5f;
    WetSeasonPeakParams.SunColor = FLinearColor(0.9f, 1.0f, 1.1f, 1.0f);
    WetSeasonPeakParams.AtmosphereHumidity = 0.9f;
    WetSeasonPeakParams.CloudCoverage = 0.7f;
    WetSeasonPeakParams.VegetationDensity = 1.3f;
    WetSeasonPeakParams.VegetationTint = FLinearColor(0.5f, 1.2f, 0.9f, 1.0f);
    WetSeasonPeakParams.WaterLevel = 10.0f;
    WetSeasonPeakParams.AverageTemperature = 25.0f;
    WetSeasonPeakParams.RainfallProbability = 0.7f;
    
    // Set initial parameters
    CurrentParameters = GetSeasonParameters(CurrentSeason);
}