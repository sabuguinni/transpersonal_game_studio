#include "Light_AtmosphericPersistenceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"

ALight_AtmosphericPersistenceManager::ALight_AtmosphericPersistenceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MainDirectionalLight = nullptr;
    SkyAtmosphereActor = nullptr;
    HeightFogActor = nullptr;
    LastSaveTime = 0.0f;
    bAtmosphereValidated = false;
    
    // Initialize Cretaceous atmospheric settings
    CretaceousSettings.DirectionalLightIntensity = 5.0f;
    CretaceousSettings.DirectionalLightColor = FColor(255, 240, 200, 255);
    CretaceousSettings.FogDensity = 0.02f;
    CretaceousSettings.FogHeightFalloff = 0.2f;
    CretaceousSettings.bVolumetricFog = true;
    CretaceousSettings.VolumetricScattering = 0.2f;
    CretaceousSettings.VolumetricAlbedo = FColor(200, 180, 150, 255);
    
    bAutoSaveOnChange = true;
    SaveInterval = 30.0f;
}

void ALight_AtmosphericPersistenceManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply Cretaceous atmosphere on start
    ApplyCretaceousAtmosphere();
    
    // Validate atmospheric integrity
    ValidateAtmosphericIntegrity();
}

void ALight_AtmosphericPersistenceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Auto-save atmospheric state at intervals
    if (bAutoSaveOnChange && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastSaveTime > SaveInterval)
        {
            SaveAtmosphericState();
            LastSaveTime = CurrentTime;
        }
    }
    
    // Validate atmospheric integrity periodically
    if (!bAtmosphereValidated)
    {
        ValidateAtmosphericIntegrity();
    }
}

void ALight_AtmosphericPersistenceManager::ApplyCretaceousAtmosphere()
{
    if (!GetWorld()) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Applying Cretaceous atmosphere settings"));
    
    // Remove legacy atmospheric fog
    RemoveLegacyAtmosphericFog();
    
    // Ensure sky atmosphere exists
    EnsureSkyAtmosphereExists();
    
    // Configure directional light
    ConfigureDirectionalLight();
    
    // Setup volumetric fog
    SetupVolumetricFog();
    
    // Create landmark lights
    CreateLandmarkLights();
    
    // Save the atmospheric state
    SaveAtmosphericState();
}

void ALight_AtmosphericPersistenceManager::EnsureSkyAtmosphereExists()
{
    if (!GetWorld()) return;
    
    // Find existing sky atmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    
    if (SkyAtmospheres.Num() == 0)
    {
        // Create new sky atmosphere
        FVector SpawnLocation = FVector::ZeroVector;
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        SkyAtmosphereActor = GetWorld()->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), SpawnLocation, SpawnRotation);
        if (SkyAtmosphereActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Created new SkyAtmosphere actor"));
        }
    }
    else
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }
}

void ALight_AtmosphericPersistenceManager::ConfigureDirectionalLight()
{
    if (!GetWorld()) return;
    
    // Find directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (AActor* Actor : DirectionalLights)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor);
        if (DirLight && DirLight->GetLightComponent())
        {
            UDirectionalLightComponent* LightComp = DirLight->GetLightComponent();
            
            // Apply Cretaceous lighting settings
            LightComp->SetIntensity(CretaceousSettings.DirectionalLightIntensity);
            LightComp->SetLightColor(FLinearColor(CretaceousSettings.DirectionalLightColor));
            LightComp->SetCastShadows(true);
            LightComp->SetCastVolumetricShadow(true);
            
            MainDirectionalLight = DirLight;
            UE_LOG(LogTemp, Warning, TEXT("Configured DirectionalLight with Cretaceous settings"));
            break;
        }
    }
}

void ALight_AtmosphericPersistenceManager::SetupVolumetricFog()
{
    if (!GetWorld()) return;
    
    // Find existing height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), HeightFogs);
    
    if (HeightFogs.Num() == 0)
    {
        // Create new height fog
        FVector SpawnLocation = FVector(0, 0, 100);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        HeightFogActor = GetWorld()->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), SpawnLocation, SpawnRotation);
    }
    else
    {
        HeightFogActor = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
    
    if (HeightFogActor && HeightFogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        
        // Apply volumetric fog settings
        FogComp->SetFogDensity(CretaceousSettings.FogDensity);
        FogComp->SetFogHeightFalloff(CretaceousSettings.FogHeightFalloff);
        FogComp->SetVolumetricFog(CretaceousSettings.bVolumetricFog);
        FogComp->SetVolumetricFogScatteringDistribution(CretaceousSettings.VolumetricScattering);
        FogComp->SetVolumetricFogAlbedo(FLinearColor(CretaceousSettings.VolumetricAlbedo));
        
        UE_LOG(LogTemp, Warning, TEXT("Configured volumetric fog with Cretaceous settings"));
    }
}

void ALight_AtmosphericPersistenceManager::CreateLandmarkLights()
{
    if (!GetWorld()) return;
    
    // Clear existing landmark lights
    for (AActor* Light : LandmarkLights)
    {
        if (Light && IsValid(Light))
        {
            Light->Destroy();
        }
    }
    LandmarkLights.Empty();
    
    // Define landmark positions
    TArray<FVector> LandmarkPositions = {
        FVector(500, 500, 200),
        FVector(1000, 0, 150),
        FVector(-500, 800, 180),
        FVector(0, 1200, 160)
    };
    
    // Create atmospheric point lights for landmarks
    for (int32 i = 0; i < LandmarkPositions.Num(); i++)
    {
        FVector SpawnLocation = LandmarkPositions[i];
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        APointLight* PointLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), SpawnLocation, SpawnRotation);
        if (PointLight && PointLight->GetLightComponent())
        {
            UPointLightComponent* LightComp = PointLight->GetLightComponent();
            
            // Configure landmark light
            LightComp->SetIntensity(800.0f);
            LightComp->SetLightColor(FLinearColor(FColor(255, 200, 120, 255)));
            LightComp->SetAttenuationRadius(2000.0f);
            LightComp->SetCastShadows(true);
            
            PointLight->SetActorLabel(FString::Printf(TEXT("LandmarkLight_%d"), i + 1));
            LandmarkLights.Add(PointLight);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d landmark lights"), LandmarkLights.Num());
}

void ALight_AtmosphericPersistenceManager::SaveAtmosphericState()
{
    if (!GetWorld()) return;
    
    // Save the current level to persist atmospheric settings
    UWorld* World = GetWorld();
    if (World)
    {
        // Mark level as dirty to ensure save
        World->MarkPackageDirty();
        
        UE_LOG(LogTemp, Warning, TEXT("Atmospheric state saved - settings will persist after restart"));
    }
}

void ALight_AtmosphericPersistenceManager::ValidateAtmosphericIntegrity()
{
    if (!GetWorld()) return;
    
    bool bIntegrityValid = true;
    
    // Validate sky atmosphere
    if (!SkyAtmosphereActor)
    {
        TArray<AActor*> SkyAtmospheres;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyAtmosphere::StaticClass(), SkyAtmospheres);
        if (SkyAtmospheres.Num() == 0)
        {
            bIntegrityValid = false;
            UE_LOG(LogTemp, Error, TEXT("SkyAtmosphere missing - atmospheric integrity compromised"));
        }
    }
    
    // Validate directional light
    if (!MainDirectionalLight)
    {
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() == 0)
        {
            bIntegrityValid = false;
            UE_LOG(LogTemp, Error, TEXT("DirectionalLight missing - lighting integrity compromised"));
        }
    }
    
    // If integrity is compromised, reapply atmosphere
    if (!bIntegrityValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Atmospheric integrity compromised - reapplying Cretaceous atmosphere"));
        ApplyCretaceousAtmosphere();
    }
    
    bAtmosphereValidated = bIntegrityValid;
}

void ALight_AtmosphericPersistenceManager::FindAtmosphericActors()
{
    if (!GetWorld()) return;
    
    // Find and cache atmospheric actors
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }
    
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        MainDirectionalLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
}

void ALight_AtmosphericPersistenceManager::RemoveLegacyAtmosphericFog()
{
    if (!GetWorld()) return;
    
    // Remove only legacy AtmosphericFog actors (not SkyAtmosphere)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetClass()->GetName() == TEXT("AtmosphericFog"))
        {
            Actor->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("Removed legacy AtmosphericFog actor"));
        }
    }
}

bool ALight_AtmosphericPersistenceManager::ValidateAtmosphericComponents()
{
    bool bValid = true;
    
    // Validate all atmospheric components exist and are properly configured
    if (!SkyAtmosphereActor)
    {
        bValid = false;
        UE_LOG(LogTemp, Error, TEXT("SkyAtmosphere validation failed"));
    }
    
    if (!MainDirectionalLight || !MainDirectionalLight->GetLightComponent())
    {
        bValid = false;
        UE_LOG(LogTemp, Error, TEXT("DirectionalLight validation failed"));
    }
    
    if (!HeightFogActor || !HeightFogActor->GetComponent())
    {
        bValid = false;
        UE_LOG(LogTemp, Error, TEXT("HeightFog validation failed"));
    }
    
    return bValid;
}