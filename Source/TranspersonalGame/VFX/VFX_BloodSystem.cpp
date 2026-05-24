#include "VFX_BloodSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"

UVFX_BloodSystem::UVFX_BloodSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms para performance

    // Configurações padrão
    bBloodSystemEnabled = true;
    MaxBloodStains = 50.0f;
    BloodCleanupInterval = 5.0f;
    DefaultStainDuration = 30.0f;
    BloodCleanupTimer = 0.0f;

    // Inicializar arrays
    ActiveBloodStains.Empty();
    BloodStainTimers.Empty();
}

void UVFX_BloodSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: Sistema de sangue inicializado"));

    // Carregar assets Niagara (paths temporários - serão substituídos por assets reais)
    HumanBloodEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Dust")));
    DinosaurBloodEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Dust")));
    SmallAnimalBloodEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Dust")));
    BloodStainEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Dust")));

    // Carregar sons (paths temporários)
    BloodSplatterSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/EngineSounds/WhiteNoise01")));
    BloodImpactSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/EngineSounds/WhiteNoise01")));
}

void UVFX_BloodSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bBloodSystemEnabled)
        return;

    // Actualizar timers das manchas de sangue
    BloodCleanupTimer += DeltaTime;
    
    for (int32 i = 0; i < BloodStainTimers.Num(); i++)
    {
        BloodStainTimers[i] += DeltaTime;
    }

    // Limpeza periódica
    if (BloodCleanupTimer >= BloodCleanupInterval)
    {
        CleanupOldBloodStains();
        BloodCleanupTimer = 0.0f;
    }
}

void UVFX_BloodSystem::SpawnBloodEffect(const FVector& Location, const FVFX_BloodConfig& BloodConfig)
{
    if (!bBloodSystemEnabled)
        return;

    ValidateBloodLocation(Location);

    // Spawn do efeito principal de sangue
    UNiagaraSystem* BloodSystem = GetBloodEffectForType(BloodConfig.BloodType);
    if (BloodSystem)
    {
        UNiagaraComponent* BloodComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            BloodSystem,
            Location,
            FRotator::ZeroRotator,
            FVector(BloodConfig.BloodIntensity),
            true,
            true,
            ENCPoolMethod::None,
            true
        );

        if (BloodComponent)
        {
            // Configurar parâmetros do sistema Niagara
            BloodComponent->SetFloatParameter(TEXT("Intensity"), BloodConfig.BloodIntensity);
            BloodComponent->SetVectorParameter(TEXT("Direction"), BloodConfig.BloodDirection);
            
            UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: Efeito de sangue spawned em %s"), *Location.ToString());
        }
    }

    // Criar mancha se configurado
    if (BloodConfig.bShouldStainSurface)
    {
        CreateBloodStain(Location, BloodConfig.BloodIntensity, BloodConfig.StainDuration);
    }

    // Tocar som
    PlayBloodSound(Location, BloodConfig.BloodType);
}

void UVFX_BloodSystem::SpawnBloodSplatter(const FVector& Location, const FVector& Direction, float Intensity)
{
    FVFX_BloodConfig Config;
    Config.BloodType = EVFX_BloodType::Human;
    Config.BloodIntensity = Intensity;
    Config.BloodDirection = Direction.GetSafeNormal();
    Config.bShouldStainSurface = true;
    Config.StainDuration = DefaultStainDuration;

    SpawnBloodEffect(Location, Config);
}

void UVFX_BloodSystem::CreateBloodStain(const FVector& Location, float StainSize, float Duration)
{
    if (!bBloodSystemEnabled || !BloodStainEffect.IsValid())
        return;

    // Verificar limite de manchas
    if (ActiveBloodStains.Num() >= MaxBloodStains)
    {
        RemoveOldestBloodStain();
    }

    // Spawn da mancha de sangue
    UNiagaraComponent* StainComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        BloodStainEffect.Get(),
        Location,
        FRotator::ZeroRotator,
        FVector(StainSize),
        true,
        false, // Não auto-destruir
        ENCPoolMethod::None,
        true
    );

    if (StainComponent)
    {
        StainComponent->SetFloatParameter(TEXT("StainSize"), StainSize);
        
        ActiveBloodStains.Add(StainComponent);
        BloodStainTimers.Add(0.0f);

        UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: Mancha de sangue criada (Total: %d)"), ActiveBloodStains.Num());
    }
}

void UVFX_BloodSystem::PlayBloodSound(const FVector& Location, EVFX_BloodType BloodType)
{
    if (!bBloodSystemEnabled)
        return;

    USoundBase* SoundToPlay = nullptr;

    // Escolher som baseado no tipo
    switch (BloodType)
    {
        case EVFX_BloodType::Human:
        case EVFX_BloodType::SmallAnimal:
            SoundToPlay = BloodSplatterSound.Get();
            break;
        case EVFX_BloodType::Dinosaur:
            SoundToPlay = BloodImpactSound.Get();
            break;
    }

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            Location,
            1.0f, // Volume
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation
            nullptr, // Concurrency
            GetOwner()
        );

        UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: Som de sangue reproduzido em %s"), *Location.ToString());
    }
}

void UVFX_BloodSystem::CleanupOldBloodStains()
{
    if (!bBloodSystemEnabled)
        return;

    TArray<int32> IndicesToRemove;

    // Encontrar manchas expiradas
    for (int32 i = 0; i < BloodStainTimers.Num(); i++)
    {
        if (BloodStainTimers[i] >= DefaultStainDuration)
        {
            IndicesToRemove.Add(i);
        }
    }

    // Remover manchas expiradas (em ordem reversa para manter índices válidos)
    for (int32 i = IndicesToRemove.Num() - 1; i >= 0; i--)
    {
        int32 IndexToRemove = IndicesToRemove[i];
        
        if (ActiveBloodStains.IsValidIndex(IndexToRemove) && IsValid(ActiveBloodStains[IndexToRemove]))
        {
            ActiveBloodStains[IndexToRemove]->DestroyComponent();
        }

        ActiveBloodStains.RemoveAt(IndexToRemove);
        BloodStainTimers.RemoveAt(IndexToRemove);
    }

    if (IndicesToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: %d manchas de sangue removidas (Restantes: %d)"), 
               IndicesToRemove.Num(), ActiveBloodStains.Num());
    }
}

void UVFX_BloodSystem::SetBloodSystemEnabled(bool bEnabled)
{
    bBloodSystemEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Limpar todas as manchas activas
        for (UNiagaraComponent* StainComponent : ActiveBloodStains)
        {
            if (IsValid(StainComponent))
            {
                StainComponent->DestroyComponent();
            }
        }
        ActiveBloodStains.Empty();
        BloodStainTimers.Empty();
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: Sistema %s"), bEnabled ? TEXT("activado") : TEXT("desactivado"));
}

UNiagaraSystem* UVFX_BloodSystem::GetBloodEffectForType(EVFX_BloodType BloodType)
{
    switch (BloodType)
    {
        case EVFX_BloodType::Human:
            return HumanBloodEffect.Get();
        case EVFX_BloodType::Dinosaur:
            return DinosaurBloodEffect.Get();
        case EVFX_BloodType::SmallAnimal:
            return SmallAnimalBloodEffect.Get();
        default:
            return HumanBloodEffect.Get();
    }
}

void UVFX_BloodSystem::ValidateBloodLocation(const FVector& Location)
{
    // Verificar se a localização está dentro dos limites do mapa
    const float MapSizeX = 157000.0f;
    const float MapSizeY = 153000.0f;
    const float HalfSizeX = MapSizeX * 0.5f;
    const float HalfSizeY = MapSizeY * 0.5f;

    if (FMath::Abs(Location.X) > HalfSizeX || FMath::Abs(Location.Y) > HalfSizeY)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_BloodSystem: Localização fora dos limites do mapa: %s"), *Location.ToString());
    }

    // Verificar se está no chão (Z não muito alto)
    if (Location.Z > 2000.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_BloodSystem: Localização muito alta para sangue: %s"), *Location.ToString());
    }
}

void UVFX_BloodSystem::RemoveOldestBloodStain()
{
    if (ActiveBloodStains.Num() > 0)
    {
        if (IsValid(ActiveBloodStains[0]))
        {
            ActiveBloodStains[0]->DestroyComponent();
        }
        
        ActiveBloodStains.RemoveAt(0);
        BloodStainTimers.RemoveAt(0);

        UE_LOG(LogTemp, Log, TEXT("VFX_BloodSystem: Mancha mais antiga removida por limite"));
    }
}