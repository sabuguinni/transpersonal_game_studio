#include "Audio_AdaptiveAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "../Core/TranspersonalGameState.h"
#include "../Core/TranspersonalCharacter.h"

AAudio_AdaptiveAudioManager::AAudio_AdaptiveAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    TransitionDuration = 2.0f;
    BiomeDetectionRadius = 1000.0f;
    DinosaurProximityRadius = 2000.0f;
    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    ProximityAlertVolume = 0.9f;
    CurrentBiome = EBiomeType::Savana;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize audio components
    InitializeAudioComponents();
    SetupProximityDetection();
}

void AAudio_AdaptiveAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache references
    PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    GameStateRef = Cast<ATranspersonalGameState>(UGameplayStatics::GetGameState(this));

    // Start with savana audio (default spawn biome)
    SwitchToBiomeAudio(EBiomeType::Savana);

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioManager: System initialized"));
}

void AAudio_AdaptiveAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle audio transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

        if (TransitionFromComponent && TransitionToComponent)
        {
            float FromVolume = FMath::Lerp(AmbienceVolume, 0.0f, TransitionProgress);
            float ToVolume = FMath::Lerp(0.0f, AmbienceVolume, TransitionProgress);

            TransitionFromComponent->SetVolumeMultiplier(FromVolume * MasterVolume);
            TransitionToComponent->SetVolumeMultiplier(ToVolume * MasterVolume);
        }

        if (TransitionProgress >= 1.0f)
        {
            bIsTransitioning = false;
            if (TransitionFromComponent)
            {
                TransitionFromComponent->Stop();
            }
        }
    }

    // Detect biome changes every 2 seconds
    static float BiomeCheckTimer = 0.0f;
    BiomeCheckTimer += DeltaTime;
    if (BiomeCheckTimer >= 2.0f)
    {
        DetectCurrentBiome();
        CheckDinosaurProximity();
        BiomeCheckTimer = 0.0f;
    }

    // Update audio based on environmental factors
    UpdateAudioBasedOnTimeOfDay();
    UpdateAudioBasedOnWeather();
}

void AAudio_AdaptiveAudioManager::InitializeAudioComponents()
{
    // Create audio components for each biome
    SavanaAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SavanaAudio"));
    SavanaAudioComponent->SetupAttachment(RootComponent);
    SavanaAudioComponent->bAutoActivate = false;

    FlorestaAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FlorestaAudio"));
    FlorestaAudioComponent->SetupAttachment(RootComponent);
    FlorestaAudioComponent->bAutoActivate = false;

    PantanoAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PantanoAudio"));
    PantanoAudioComponent->SetupAttachment(RootComponent);
    PantanoAudioComponent->bAutoActivate = false;

    DesertoAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DesertoAudio"));
    DesertoAudioComponent->SetupAttachment(RootComponent);
    DesertoAudioComponent->bAutoActivate = false;

    MontanhaAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MontanhaAudio"));
    MontanhaAudioComponent->SetupAttachment(RootComponent);
    MontanhaAudioComponent->bAutoActivate = false;
}

void AAudio_AdaptiveAudioManager::SetupProximityDetection()
{
    ProximityDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityDetection"));
    ProximityDetectionSphere->SetupAttachment(RootComponent);
    ProximityDetectionSphere->SetSphereRadius(DinosaurProximityRadius);
    ProximityDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProximityDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProximityDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAudio_AdaptiveAudioManager::DetectCurrentBiome()
{
    if (!PlayerCharacter)
        return;

    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    EBiomeType DetectedBiome = GetBiomeAtLocation(PlayerLocation);

    if (DetectedBiome != CurrentBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioManager: Biome changed from %d to %d"), 
               (int32)CurrentBiome, (int32)DetectedBiome);
        SwitchToBiomeAudio(DetectedBiome);
    }
}

EBiomeType AAudio_AdaptiveAudioManager::GetBiomeAtLocation(const FVector& Location)
{
    // Biome detection based on coordinates from brain memories
    float X = Location.X;
    float Y = Location.Y;

    // Pantano (sudoeste): X(-77500 a -25000), Y(-76500 a -15000)
    if (X >= -77500.0f && X <= -25000.0f && Y >= -76500.0f && Y <= -15000.0f)
        return EBiomeType::Pantano;

    // Floresta (noroeste): X(-77500 a -15000), Y(15000 a 76500)
    if (X >= -77500.0f && X <= -15000.0f && Y >= 15000.0f && Y <= 76500.0f)
        return EBiomeType::Floresta;

    // Deserto (leste): X(25000 a 79500), Y(-30000 a 30000)
    if (X >= 25000.0f && X <= 79500.0f && Y >= -30000.0f && Y <= 30000.0f)
        return EBiomeType::Deserto;

    // Montanha Nevada (nordeste): X(15000 a 79500), Y(20000 a 76500)
    if (X >= 15000.0f && X <= 79500.0f && Y >= 20000.0f && Y <= 76500.0f)
        return EBiomeType::MontanhaNevada;

    // Savana (centro) - default
    return EBiomeType::Savana;
}

void AAudio_AdaptiveAudioManager::SwitchToBiomeAudio(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiome && !bIsTransitioning)
        return;

    UAudioComponent* NewAudioComponent = nullptr;

    switch (NewBiome)
    {
        case EBiomeType::Savana:
            NewAudioComponent = SavanaAudioComponent;
            break;
        case EBiomeType::Floresta:
            NewAudioComponent = FlorestaAudioComponent;
            break;
        case EBiomeType::Pantano:
            NewAudioComponent = PantanoAudioComponent;
            break;
        case EBiomeType::Deserto:
            NewAudioComponent = DesertoAudioComponent;
            break;
        case EBiomeType::MontanhaNevada:
            NewAudioComponent = MontanhaAudioComponent;
            break;
    }

    if (NewAudioComponent)
    {
        CrossfadeAudio(GetCurrentBiomeAudioComponent(), NewAudioComponent);
        CurrentBiome = NewBiome;
    }
}

UAudioComponent* AAudio_AdaptiveAudioManager::GetCurrentBiomeAudioComponent()
{
    switch (CurrentBiome)
    {
        case EBiomeType::Savana: return SavanaAudioComponent;
        case EBiomeType::Floresta: return FlorestaAudioComponent;
        case EBiomeType::Pantano: return PantanoAudioComponent;
        case EBiomeType::Deserto: return DesertoAudioComponent;
        case EBiomeType::MontanhaNevada: return MontanhaAudioComponent;
        default: return SavanaAudioComponent;
    }
}

void AAudio_AdaptiveAudioManager::CrossfadeAudio(UAudioComponent* FromComponent, UAudioComponent* ToComponent)
{
    if (!ToComponent)
        return;

    TransitionFromComponent = FromComponent;
    TransitionToComponent = ToComponent;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;

    // Start new audio component
    ToComponent->SetVolumeMultiplier(0.0f);
    ToComponent->Play();
}

void AAudio_AdaptiveAudioManager::CheckDinosaurProximity()
{
    if (!PlayerCharacter)
        return;

    // Get all overlapping actors in proximity sphere
    TArray<AActor*> OverlappingActors;
    ProximityDetectionSphere->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        // Check if actor is a dinosaur (simplified check by name)
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")) || 
            ActorName.Contains(TEXT("Triceratops")) || ActorName.Contains(TEXT("Brachiosaurus")))
        {
            float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), Actor->GetActorLocation());
            
            // Determine species based on name
            EDinosaurSpecies Species = EDinosaurSpecies::TRex;
            if (ActorName.Contains(TEXT("Raptor"))) Species = EDinosaurSpecies::Raptor;
            else if (ActorName.Contains(TEXT("Triceratops"))) Species = EDinosaurSpecies::Triceratops;
            else if (ActorName.Contains(TEXT("Brachiosaurus"))) Species = EDinosaurSpecies::Brachiosaurus;

            TriggerProximityAlert(Distance, Species);
            break; // Only alert for closest dinosaur
        }
    }
}

void AAudio_AdaptiveAudioManager::TriggerProximityAlert(float Distance, EDinosaurSpecies Species)
{
    // Calculate alert intensity based on distance
    float AlertIntensity = FMath::Clamp(1.0f - (Distance / DinosaurProximityRadius), 0.0f, 1.0f);
    
    if (AlertIntensity > 0.3f) // Only alert if dinosaur is relatively close
    {
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioManager: Dinosaur proximity alert! Species: %d, Distance: %.1f, Intensity: %.2f"), 
               (int32)Species, Distance, AlertIntensity);
        
        // Play proximity alert sound with intensity-based volume
        // This would trigger MetaSound or audio cue for proximity alerts
    }
}

void AAudio_AdaptiveAudioManager::UpdateAudioBasedOnTimeOfDay()
{
    if (!GameStateRef)
        return;

    // Adjust audio based on time of day
    float TimeOfDay = GameStateRef->GetTimeOfDay();
    float NightVolume = 1.0f;

    // Reduce volume during night (22:00 - 06:00)
    if (TimeOfDay > 22.0f || TimeOfDay < 6.0f)
    {
        NightVolume = 0.6f;
    }

    // Apply night volume modifier to all biome audio components
    if (SavanaAudioComponent) SavanaAudioComponent->SetVolumeMultiplier(AmbienceVolume * NightVolume * MasterVolume);
    if (FlorestaAudioComponent) FlorestaAudioComponent->SetVolumeMultiplier(AmbienceVolume * NightVolume * MasterVolume);
    if (PantanoAudioComponent) PantanoAudioComponent->SetVolumeMultiplier(AmbienceVolume * NightVolume * MasterVolume);
    if (DesertoAudioComponent) DesertoAudioComponent->SetVolumeMultiplier(AmbienceVolume * NightVolume * MasterVolume);
    if (MontanhaAudioComponent) MontanhaAudioComponent->SetVolumeMultiplier(AmbienceVolume * NightVolume * MasterVolume);
}

void AAudio_AdaptiveAudioManager::UpdateAudioBasedOnWeather()
{
    if (!GameStateRef)
        return;

    // Adjust audio based on weather conditions
    EWeatherType CurrentWeather = GameStateRef->GetCurrentWeather();
    float WeatherVolumeModifier = 1.0f;

    switch (CurrentWeather)
    {
        case EWeatherType::Storm:
            WeatherVolumeModifier = 1.5f; // Louder during storms
            break;
        case EWeatherType::Rain:
            WeatherVolumeModifier = 1.2f; // Slightly louder during rain
            break;
        case EWeatherType::Fog:
            WeatherVolumeModifier = 0.8f; // Muffled during fog
            break;
        default:
            WeatherVolumeModifier = 1.0f;
            break;
    }

    // Apply weather modifier (this would be more sophisticated in practice)
    MasterVolume = FMath::Clamp(MasterVolume * WeatherVolumeModifier, 0.1f, 2.0f);
}

void AAudio_AdaptiveAudioManager::UpdateAudioZones()
{
    // Update audio zones based on player position and biome
    DetectCurrentBiome();
}

void AAudio_AdaptiveAudioManager::SetBiomeAudioVolume(EBiomeType Biome, float Volume)
{
    UAudioComponent* TargetComponent = nullptr;

    switch (Biome)
    {
        case EBiomeType::Savana: TargetComponent = SavanaAudioComponent; break;
        case EBiomeType::Floresta: TargetComponent = FlorestaAudioComponent; break;
        case EBiomeType::Pantano: TargetComponent = PantanoAudioComponent; break;
        case EBiomeType::Deserto: TargetComponent = DesertoAudioComponent; break;
        case EBiomeType::MontanhaNevada: TargetComponent = MontanhaAudioComponent; break;
    }

    if (TargetComponent)
    {
        TargetComponent->SetVolumeMultiplier(Volume * MasterVolume);
    }
}